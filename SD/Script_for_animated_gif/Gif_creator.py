from PIL import Image
import os
print(f"Stacking images")
image_folder = "Pictures"
images = sorted([img for img in os.listdir(image_folder) if img.lower().endswith('.png')])
image_sequence = [Image.open(os.path.join(image_folder, img)) for img in images]
if not image_sequence:
    raise ValueError("No PNG images in folder")
output_path = "Animation.gif"
image_sequence[0].save(
    output_path,
    save_all=True,
    append_images=image_sequence[1:],
    duration=50,   
    loop=0         
)

print(f"Animation made: {output_path}")
