clc
clear

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load image
target_png_file='Output_averaged.png'; %target file for averaged image
disp(['Deleting image ',target_png_file,'...'])
delete(target_png_file)
frame=[];
listing = dir('*.png');
for k=1:1:length(listing)
    currentfilename = listing(k).name;
    disp(['Converting image ',currentfilename,' in progress...'])
    [a,map]=imread(currentfilename);
    if not(isempty(map));%dealing with indexed images
        disp('Indexed image, converting to grayscale');
        a=ind2gray(a,map);
    end
    frame(:,:,k)=double(a(:,:,1));
end

averaged_output=uint8(mean(frame,3));
imshow(averaged_output);
disp(['Writing image ',target_png_file,'...'])
imwrite(averaged_output,target_png_file)
disp('End of conversion, enjoy your fancy output !')
