% Raphael BOICHOT 20/01/2025 thermal printer emulator for the TinyGB Printer
% multi OS compatibility improved by Cristofer Cruz 2022/06/21
% Compatible with Matlab (requires additional library) and Octave natively
% This code can take any image as input as long as it is PNG
clear
clc
close all

%%%%%%%%%%%%%%%%%%%%%%%%%General parameters%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
paper_color=2; %6=random, 5=purple, 4=pink, 3=regular blue, 2=regular yellow or 1=regular white
darkness=8; %1=lightest 10=darkest
scale_percentage=25; %100=full size, smaller values scale down image
%%%%%%%%%%%%%%%%%%%%%%%%%General parameters%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

try
    pkg load image % for compatibility with Octave
catch
    % Nothing to do
end

mkdir Paper_out
imagefiles = dir('Image_in/*.png');% the default format is png, other are ignored
nfiles = length(imagefiles);    % Number of files found

for k=1:1:nfiles
    currentfilename = imagefiles(k).name;
    disp(['Converting image ',currentfilename,' in progress...'])
    [a,map]=imread(['Image_in/',currentfilename]);
    if not(isempty(map));%dealing with indexed images
        disp('Indexed image, converting to grayscale');
        a=ind2gray(a,map);
    end

    imshow(a)
    drawnow

    [height, width, layers]=size(a);
    if layers>1%dealing with color images
        disp('Color image, converting to grayscale');
        a=rgb2gray(a);
        [height, width, layers]=size(a);
    end
    C=unique(a);

    if (length(C)<=4 && height==160);%dealing with pixel perfect image, bad orientation
        disp('Bad orientation, image rotated');
        a=imrotate(a,270);
        [heigth, width,layers]=size(a);
    end

    if (length(C)<=4 && not(width==160));%dealing with pixel perfect upscaled/downscaled images
        disp('Image is 2 bpp or less, but width has to be rescaled');
        a=imresize(a,160/width,"nearest");
        [heigth, width,layers]=size(a);
    end

    if (length(C)>4 || not(width==160));%dealing with 8-bit images in general
        disp('8-bits image rectified and dithered with Bayer matrices');
        a=image_rectifier(a);
        [height, width, layers]=size(a);
    end

    if length(C)==1;%dealing with one color images
        disp('Empty image -> neutralization, will print full white');
        a=zeros(height, width);
    end

    if not(rem(height,16)==0);%Fixing images not multiple of 16 pixels
        disp('Image height is not a multiple of 16 : fixing image');
        C=unique(a);
        new_lines=ceil(height/16)*16-height;
        color_footer=double(C(end));
        footer=color_footer.*ones(new_lines,width, layers);
        a=[a;footer];
        [height, width, layers]=size(a);
    end

    [height, width, layers]=size(a);
    C=unique(a);
    switch length(C)
        case 4; a=(a==C(1))*0+(a==C(2))*84+(a==C(3))*168+(a==C(4))*255;
        case 3; a=(a==C(1))*0+(a==C(2))*84+(a==C(3))*255;
        case 2; a=(a==C(1))*0+(a==C(2))*255;
    end;
    [epaper, alpha]=epaper_packet(a,paper_color,darkness,scale_percentage);
    imshow(epaper)
    drawnow
    pause(1);
    imwrite(epaper,['Paper_out/printerPaper-dark',num2str(darkness),'-',currentfilename],'Alpha',alpha);
    disp(['printerPaper-dark',num2str(darkness),'-',currentfilename,' saved.'])
end
close all
msgbox('Done !');
disp('Done!')

