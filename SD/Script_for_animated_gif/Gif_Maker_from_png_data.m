clc
clear

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load image

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_gif_file='Animation.gif'; %target file for animated gif
gif_deadtime=0.05;               %delay is seconds between pictures for animated gifs
gif_skip=1;                      %keep every 1 out of gif_skip image for gif
scaling_factor=1;                %because you may want to change the image format which is 4x natively
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

listing = dir('./Pictures/*.png');
for i=1:1:length(listing)
    name=listing(i).name;
    disp(['Processing ',listing(i).name]);
    [frame,map]=imread(['./Pictures/',name]);
    frame=imresize(frame,scaling_factor,'nearest');
    if i==1
        imwrite(frame,map,target_gif_file,'gif', 'Loopcount',inf,'DelayTime',gif_deadtime,'Compression','bzip');
    else
        if rem(i,gif_skip)==0
        imwrite(frame,map,target_gif_file,'gif','WriteMode','append','DelayTime',gif_deadtime,'Compression','bzip');
        end
    end
end

disp('End of conversion, enjoy your fancy animations !')
