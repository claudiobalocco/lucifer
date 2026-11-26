clear all
close all
pkg load video
%read_lucifer_data
data_file = 'luc_data_ex1.dat';
view_idx = 1;  % field view to show: 1 = first entry of 'field_views' in the conf file

make_movie = 1;
movie_name = 'field_video_ex1.avi';
frame_rate = 50;
scale_f = 100; % change constrat in movie
cell_size_v1 = 5e-6; % in m, used only for files written by Lucifer <= 0.4.2

% ------- header (see src/write_header.c) ----------
% format 2 (Lucifer >= 1.0.0):
%   128 bytes text, int32 format, int32 sx sy sz, int32 # time steps,
%   float32 courant, float32 cell size (m), float32 dt (s), int32 # views,
%   per view: 2 char field, 1 char axis [x|y|z|p], 3 int32 position
% format 1 (Lucifer <= 0.4.2): text starts with 'LUCIFER v'
%   int16 sizes and # time steps, float32 courant, int16 # views,
%   per view 2 char + 1 char + int16 (the view info was not filled in correctly,
%   the data is one xz plane per time step)

fid = fopen(data_file, 'r', 'ieee-le');
if fid < 0
    error('cannot open %s', data_file);
end

c0 = 299792458;
ht = fread(fid, 128, 'uint8=>char')';
if strncmp(ht, 'LUCIFER v', 9)
    % format 1
    sz3 = fread(fid, 3, 'uint16=>double');
    maxt = fread(fid, 1, 'uint16=>double');
    cn = fread(fid, 1, 'float32=>double');
    nviews = fread(fid, 1, 'uint16=>double');
    fseek(fid, 5*nviews, 'cof');
    cell_size = cell_size_v1;
    dt = cn*cell_size/c0;
    views = struct('field', 'Ex', 'axis', 'y', 'pos', [0 0 0]);
else
    fmt = fread(fid, 1, 'int32=>double');
    if fmt ~= 2
        error('unsupported data file format %d', fmt);
    end
    sz3 = fread(fid, 3, 'int32=>double');
    maxt = fread(fid, 1, 'int32=>double');
    cn = fread(fid, 1, 'float32=>double');
    cell_size = fread(fid, 1, 'float32=>double');
    dt = fread(fid, 1, 'float32=>double');
    nviews = fread(fid, 1, 'int32=>double');
    for j=1:nviews
        views(j).field = fread(fid, 2, 'uint8=>char')';
        views(j).axis = fread(fid, 1, 'uint8=>char');
        views(j).pos = fread(fid, 3, 'int32=>double')';
    end
end
sx = sz3(1); sy = sz3(2); sz = sz3(3);

% # of values of each view per time step, and shape of the plane
for j=1:numel(views)
    switch views(j).axis
        case 'p'
            views(j).n = 1; views(j).shape = [1 1];
        case 'y' % xz plane: x outer, z inner
            views(j).n = sx*sz; views(j).shape = [sz sx];
        case 'x' % yz plane: y outer, z inner
            views(j).n = sy*sz; views(j).shape = [sz sy];
        case 'z' % xy plane: x outer, y inner
            views(j).n = sx*sy; views(j).shape = [sy sx];
    end
end
offsets = [0 cumsum([views.n])];
frame_len = offsets(end);   % floats per time step
v = views(view_idx);
printf('%s: %dx%dx%d cells, %d steps, %d view(s); showing %s (axis %c)\n', ...
    strtrim(ht(ht ~= 0)), sx, sy, sz, maxt, numel(views), v.field, v.axis);

t = (0:maxt-1)*dt*1e12; % in ps

fpos_data = ftell(fid); %check position in data file

if v.axis == 'p'
    % a point: plot the time trace
    trace = zeros(1, maxt);
    for i=1:maxt
        fseek(fid, fpos_data + 4*((i-1)*frame_len + offsets(view_idx)), SEEK_SET);
        trace(i) = fread(fid, 1, 'float32=>double');
    end
    plot(t, trace)
    xlabel('t (ps)')
    ylabel(v.field)
    fclose(fid);
    return
end

smax = 0;
for i=1:maxt
    fseek(fid, fpos_data + 4*((i-1)*frame_len + offsets(view_idx)), SEEK_SET);
    s = fread(fid, v.n, '*float32');
    ssmax = max(abs(s));
    if (ssmax > smax)
        smax = ssmax;
    end
end
if smax == 0
    smax = 1; % no field at all
end

if make_movie
    aviobj = avifile(movie_name,"fps",frame_rate);
end

for i=1:maxt
    fseek(fid, fpos_data + 4*((i-1)*frame_len + offsets(view_idx)), SEEK_SET);
    s = fread(fid, v.n, '*float32');
    s = reshape(s, v.shape);
    s = (s/(2*smax) + 0.5); %between 0 and 1; 0.5 is no field
    %image(s*32)
    %colormap gray
    %axis equal
    %drawnow

    if make_movie
        s = (s-0.5)*scale_f;
        s(s>0.5) = 0.5;
        s(s<-0.5) = -0.5;
        s = s + 0.5;
        addframe(aviobj,s);
    end
end

fclose(fid);
