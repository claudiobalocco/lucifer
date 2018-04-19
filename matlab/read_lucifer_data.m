clear all
close all
%read_lucifer_data
fid=fopen('test.dat');        % Open the file.

make_movie = 1;

if make_movie
    movie_name = 'test.avi';
    aviobj = VideoWriter(movie_name);
    aviobj.FrameRate = 50;
    open(aviobj)
end
scale_f = 10; % change constrat in movie
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

%fpos_data = ftell(fid);
%framen = 128;
%fpos_frame = (framen-1)*sx*sy;

%fseek(fid,fpos_frame,fpos_data);
%fseek(fid,5080000,147);i

for i=1:maxt
    s = fread(fid, sx*sz, '*float');
    s = reshape(s,[sz sx]);
    
    image((s+scale_f)*(1/scale_f)*32)
    colormap gray
    axis equal
    drawnow
       
    if make_movie
        fields_mv = getframe;
        writeVideo(aviobj,fields_mv);
    end
end

fclose(fid);
if make_movie
    close(aviobj);
end


