clc
clear

disp('-----------------------------------------------------------')
disp('| Progressive Averaged Grayscale GIF for Octave (bzip)    |')
disp('-----------------------------------------------------------')

pkg load image

% Parameters
target_gif_file = 'Animation_progressive.gif';
gif_deadtime    = 0.05;
gif_skip        = 9;
scaling_factor  = 1;
n_colors        = 256;

% Generate fixed grayscale colormap
gray_map = repmat((0:255)'/255, 1, 3);  % 256×3

listing = dir('./Pictures/*.png');
n_files = numel(listing);
if n_files == 0
    error("No PNG images found in ./Pictures/")
end

disp('Reading and averaging images...')

% Read first frame
[frame, map] = imread(fullfile('./Pictures', listing(1).name));
if scaling_factor ~= 1
    frame = imresize(frame, scaling_factor, 'nearest');
end

% Convert to RGB and then to grayscale
current_avg = ind2rgb(frame, map);
current_avg = rgb2gray(current_avg);  % double [0,1]

frames_idx = {};
frame_counter = 1;

% Convert first frame to indexed using fixed grayscale map
frames_idx{1} = uint8(current_avg*255);

% Process remaining frames
for i = 2:n_files
    if rem(i,gif_skip) == 0
        [frame, map] = imread(fullfile('./Pictures', listing(i).name));
        if scaling_factor ~= 1
            frame = imresize(frame, scaling_factor, 'nearest');
        end

        rgb = ind2rgb(frame, map);
        gray = rgb2gray(rgb);

        current_avg = ((frame_counter)*current_avg + gray) / (frame_counter+1);
        frame_counter += 1;

        frames_idx{end+1} = uint8(current_avg*255);

        fprintf("Processed %s (%d/%d)\n", listing(i).name, i, n_files);
    end
end

disp('Writing animated GIF...')

imwrite(frames_idx{1}, gray_map, target_gif_file, ...
        'gif', 'LoopCount', Inf, 'DelayTime', gif_deadtime, 'Compression', 'bzip');

for k = 2:numel(frames_idx)
    fprintf("Appending frame %d/%d\n", k, numel(frames_idx));
    imwrite(frames_idx{k}, gray_map, target_gif_file, ...
            'gif', 'WriteMode', 'append', 'DelayTime', gif_deadtime, 'Compression', 'bzip');
end

disp('-----------------------------------------------------------')
disp('Done! Progressive averaged grayscale GIF is ready!')
disp('-----------------------------------------------------------')

