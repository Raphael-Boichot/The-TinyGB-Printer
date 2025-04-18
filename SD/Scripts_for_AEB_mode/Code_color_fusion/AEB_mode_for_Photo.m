function []=AEB_mode_for_Photo(target_png_folder,target_png_file, color_limit)
disp(['Deleting image ',target_png_file,'...'])
delete(target_png_file)
frame=[];
listing = dir([target_png_folder,'*.png']);
m=1;
for k=1:1:length(listing)
    currentfilename = listing(k).name;
    disp(['Converting image ',currentfilename,' in progress...'])
    [a,map]=imread([target_png_folder,currentfilename]);
    if not(isempty(map));%dealing with indexed images
        disp('Indexed image, converting to grayscale');
        a=ind2gray(a,map);
    end
    [height, width, depth]=size(a);
    average_intensity=mean(mean(a(width/5:height-width/5,width/5:width-width/5)));
    reject=0;
    if average_intensity<color_limit;
        disp('Image rejected');
        reject=1;
    end
    if average_intensity>(255-color_limit);
        disp('Image rejected');
        reject=1;
    end

    if reject==0
        disp('Image stacked');
        frame(:,:,m)=double(a(:,:,1));
        m=m+1;
    end
end

averaged_output=uint8(mean(frame,3));
imshow(averaged_output);
pause(2)
disp(['Writing image ',target_png_file,'...'])
imwrite(averaged_output,target_png_file)

