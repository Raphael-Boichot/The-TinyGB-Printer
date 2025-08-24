clc
clear

disp('-----------------------------------------------------------')
disp('| Optimized Indexed GIF creator for GNU Octave (bzip)     |')
disp('-----------------------------------------------------------')

pkg load image

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_gif_file = 'Animation.gif';  % target file for animated gif
gif_deadtime    = 0.05;             % delay in seconds between pictures
gif_skip        = 1;                % keep every 1 out of gif_skip images
scaling_factor  = 1;                % scale images if needed
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Load file list
listing = dir('./Pictures/*.png');
n_files = numel(listing);

if n_files == 0
    error("No PNG images found in ./Pictures/")
end

% Preload frames + maps
frames = {};
maps   = {};
disp('Reading and resizing images...')
for i = 1:n_files
    name = listing(i).name;
    fprintf("Processing %s (%d/%d)\n", name, i, n_files);

    if rem(i,gif_skip) == 0
        [frame, map] = imread(fullfile('./Pictures', name));
        if scaling_factor ~= 1
            frame = imresize(frame, scaling_factor, 'nearest');
        end
        frames{end+1} = frame;
        maps{end+1}   = map;
    end
end

fprintf("Total frames to write: %d\n", numel(frames));

% Write GIF (with mandatory bzip compression)
disp("Writing animated GIF...")
imwrite(frames{1}, maps{1}, target_gif_file, ...
        "gif", "LoopCount", Inf, ...
        "DelayTime", gif_deadtime, ...
        "Compression", "bzip");

for i = 2:numel(frames)
    fprintf("Appending frame %d/%d\n", i, numel(frames));
    imwrite(frames{i}, maps{i}, target_gif_file, ...
            "gif", "WriteMode", "append", ...
            "DelayTime", gif_deadtime, ...
            "Compression", "bzip");
end

disp('-----------------------------------------------------------')
disp('End of conversion, enjoy your fancy animations !')
disp('-----------------------------------------------------------')

