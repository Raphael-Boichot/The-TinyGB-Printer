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
scaling_factor=1;
color_limit=4; %threshold to reject an image /255
color_weight=[1 1 1];       %[R G B] weights to get a gray image when taking a white screen in photo in my case
%to get these values, take pictures of a white scene with the three filters, note the exposure times and divide by the minimal value
%these corresponds to my filters but yours can vary
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

AEB_mode_for_Photo('./Red_pictures/','Red.png',color_limit) %processes red average
AEB_mode_for_Photo('./Green_pictures/','Green.png',color_limit) %processes green average
AEB_mode_for_Photo('./Blue_pictures/','Blue.png',color_limit) %processes blue average

delete(target_png_file)
R=imread('Red.png');
G=imread('Green.png');
B=imread('Blue.png');
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
imshow(frame)
disp('End of conversion, enjoy your fancy RGB fusion !')
