clc
clear
pkg load image
%script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%target_mp4_file='Output.mp4'; %target file for mp4, keep all image
target_gif_file='Animation.gif'; %target file for animated gif
gif_deadtime=0.05;            %delay is seconds between pictures for animated gifs
gif_skip=1;                   %keep every 1 out of gif_skip image for gif
scaling_factor=1;           %because images are 8x after powershell step
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

listing = dir(['./Pictures/','*.png']);
for i=1:1:length(listing)
    name=listing(i).name;
    disp(['Processing ',listing(i).name]);
    [frame,map]=imread(['./Pictures/',name]);
    if not(isempty(map)) %dealing with indexed images
        disp('Indexed image, converting to grayscale');
        frame=ind2gray(frame,map);
    end
    frame=imresize(frame,scaling_factor,'nearest');
    RGB=cat(3,frame,frame,frame);
    [imind,map] = rgb2ind(RGB);
    if i==1
        imwrite(imind,map,target_gif_file,'gif', 'Loopcount',inf,'DelayTime',gif_deadtime);
    else
        if rem(i,gif_skip)==0
        imwrite(imind,map,target_gif_file,'gif','WriteMode','append','DelayTime',gif_deadtime);
        end
    end
end

disp('End of conversion, enjoy your fancy animations !')
