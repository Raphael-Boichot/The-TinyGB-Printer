##image converter to 2bpp 160 pix width
function [image_rectified]=image_rectifier(image_non_rectified)
Dithering_patterns = [0x2A, 0x5E, 0x9B, 0x51, 0x8B, 0xCA, 0x33, 0x69, 0xA6, 0x5A, 0x97, 0xD6, 0x44, 0x7C, 0xBA, 0x37, 0x6D, 0xAA, 0x4D, 0x87, 0xC6, 0x40, 0x78, 0xB6, 0x30, 0x65, 0xA2, 0x57, 0x93, 0xD2, 0x2D, 0x61, 0x9E, 0x54, 0x8F, 0xCE, 0x4A, 0x84, 0xC2, 0x3D, 0x74, 0xB2, 0x47, 0x80, 0xBE, 0x3A, 0x71, 0xAE];
a=image_non_rectified;
[height, width,layers]=size(a);

if (height<width)
    a=imrotate(a,270);
end

[height, width,layers]=size(a);
a=imresize(a,160/width,"cubic");
[height, width,layers]=size(a);

if not(rem(height,16)==0);##Fixing images not multiple of 16 pixels
    disp('Image height is not a multiple of 16 : fixing image');
    new_lines=ceil(height/16)*16-height;
    color_footer=255;
    footer=color_footer.*ones(new_lines,width, layers);
    a=[a;footer];
    [height, width,layers]=size(a);
end

##2D edge enhancement
edge=double(a);
alpha=0.5;
for (y = 2:1:height-1)
    for (x = 2:1:width-1)
        b(y,x)=(4*edge(y,x)-edge(y-1,x)-edge(y+1,x)-edge(y,x-1)-edge(y,x+1)).*alpha;
    end
end
a(1:height-1,1:width-1)=uint8(double(a(1:height-1,1:width-1))+b);
##Bayer dithering (what a Game Boy Camera does)
Bayer_matDG_B=[];
Bayer_matLG_DG=[];
Bayer_matW_LG=[];

counter = 1;
for (y = 1:1:4)
    for (x = 1:1:4)
        Bayer_matDG_B(y,x) = Dithering_patterns(counter);
        counter = counter + 1;
        Bayer_matLG_DG(y,x) = Dithering_patterns(counter);
        counter = counter + 1;
        Bayer_matW_LG(y,x) = Dithering_patterns(counter);
        counter = counter + 1;
    end
end

for (y = 1:4:height)
    for (x = 1:4:width)
        Bayer_matDG_B_2D(y:y+3,x:x+3)=Bayer_matDG_B;
        Bayer_matLG_DG_2D(y:y+3,x:x+3)=Bayer_matLG_DG;
        Bayer_matW_LG_2D(y:y+3,x:x+3)=Bayer_matW_LG;
    end
end

for (y = 1:1:height)
    for (x = 1:1:width)
        pixel = a(y,x);
        if (pixel < Bayer_matDG_B_2D(y,x));
            pixel_out = 0;
        end
        if ((pixel >= Bayer_matDG_B_2D(y,x)) && (pixel < Bayer_matLG_DG_2D(y,x)));
            pixel_out = 80;
        end
        if ((pixel >= Bayer_matLG_DG_2D(y,x)) && (pixel < Bayer_matW_LG_2D(y,x)));
            pixel_out = 170;
        end
        if (pixel >= Bayer_matW_LG_2D(y,x));
            pixel_out = 255;
        end
        a(y,x) = pixel_out;
    end
end

image_rectified=(a);

