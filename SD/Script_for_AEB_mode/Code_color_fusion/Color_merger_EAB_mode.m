%images must be taken with color filter, in this order: RGBRGBRGB...
%the exposure must be set to auto all the time and the DashBoy allowed to
%adjust exposure between each filter.
clc
clear

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load image

%script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_png_file='Color_fusion.png'; %target file for animated gif
scaling_factor=1;          %because images are 8x after powershell step
color_weight=[1 1 1]        %[R G B] weights to get a gray image when taking a white screen in photo in my case
%to get these values, take pictures of a white scene with the three filters, note the exposure times and divide by the minimal value
%these corresponds to my filters but yours can vary
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
delete(target_png_file)
listing = dir('*.png');
gif_counter=0;
for i=1:3:length(listing)
    gif_counter=gif_counter+1;
    name=listing(i).name;
    disp(['Processing ',listing(i).name]);
    R=imread(name);
    name=listing(i+1).name;
    disp(['Processing ',listing(i+1).name]);
    G=imread(name);
    name=listing(i+2).name;
    disp(['Processing ',listing(i+2).name]);
    B=imread(name);
    [height, width, null]=size(R);
    R=imresize(R,scaling_factor,'nearest');
    G=imresize(G,scaling_factor,'nearest');
    B=imresize(B,scaling_factor,'nearest');
    R=uint8(R(:,:,1)*color_weight(1));
    G=uint8(G(:,:,1)*color_weight(2));
    B=uint8(B(:,:,1)*color_weight(3));
    frame(:,:,1)=R;
    frame(:,:,2)=G;
    frame(:,:,3)=B;
    imwrite(frame,target_png_file);
end
disp('End of conversion, enjoy your fancy RGB fusion !')
