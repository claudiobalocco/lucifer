clear all
close all
pkg load video
%read_lucifer_data
data_file = 'luc_data_ex1.dat';
fid=fopen(data_file);        % Open the file.

make_movie = 1;

if make_movie
    movie_name = 'field_video_ex1.avi';
    frame_rate = 50;
    aviobj = avifile(movie_name,"fps",frame_rate);
    %open(aviobj)
end
scale_f = 100; % change constrat in movie
cell_size = 5; %in um

%! ------- header structure ----------
%! ------ free text (128 bytes)-------
%!
%! ------- sim size (3*2 bytes)-------
%! 3 integer*2 SizeX, SizeY, and SizeZ
%! ------- max time (2 bytes) --------
%! integer*2
%! ----- courant number (4 bytes)-----
%! real*4
%! ------- # of slices (2 bytes) -----
%! 1 integer*2
%! -------- 1 slice (5 bytes) --------
%! 1 char [x|y|z] + 2 char field + location (integer*2)
%! ------------- [...] ---------------
%! ----------- data start ------------

ht = fread(fid, 128, 'uint8=>char');
sx = fread(fid, 1,'int16=>int32');
sy = fread(fid, 1,'int16=>int32');
sz = fread(fid, 1,'int16=>int32');
maxt = fread(fid, 1,'int16=>int32');
cn = fread(fid, 1, 'float');
nslices = fread(fid, 1,'int16=>int32');



for j=1:nslices
    field = fread(fid, 2, 'uint8=>char');
    ax = fread(fid, 1, 'uint8=>char');
    pos = fread(fid, 1,'int16=>int32');
end

c0=3e8;
dt = double(cn)*cell_size*1e-6/c0;
sim_time=dt*(double(maxt)-1);
t=(0:dt:sim_time)*1e12; %in ps

fpos_data = ftell(fid); %check position in data file

smax = 0;
for i=1:maxt
    s = fread(fid, sx*sz, '*float');
    ssmax = max(max(abs(s)));
    if (ssmax > smax)
        smax = ssmax;
    end
end

fseek(fid,fpos_data); % rewind the data file

for i=1:maxt
    s = fread(fid, sx*sz, '*float');
    s = reshape(s,[sz sx]);
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
if make_movie
    %close(aviobj);
end


