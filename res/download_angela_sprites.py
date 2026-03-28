import os
import requests
import time

# Directory to save the sprite frames
output_dir = r"D:\develop\Qtproject\honor-of-hero\res\angela_sprites"

# Create the directory if it doesn't exist
os.makedirs(output_dir, exist_ok=True)

# Hypothetical URLs for Angela's sprite frames (these are examples only)
# In a real scenario, you would get these from a legitimate API or resource
sprite_urls = [
    # These are placeholder URLs - replace with actual sprite frame URLs
    # "https://example.com/angela/frame_001.png",
    # "https://example.com/angela/frame_002.png",
    # "https://example.com/angela/frame_003.png",
]

# Function to download a single sprite
def download_sprite(url, filename):
    try:
        print(f"Downloading {filename}...")
        response = requests.get(url)
        if response.status_code == 200:
            filepath = os.path.join(output_dir, filename)
            with open(filepath, 'wb') as f:
                f.write(response.content)
            print(f"Saved {filename}")
        else:
            print(f"Failed to download {filename}: Status {response.status_code}")
    except Exception as e:
        print(f"Error downloading {filename}: {str(e)}")

# Download all sprites
for i, url in enumerate(sprite_urls):
    filename = f"angela_frame_{i+1:03d}.png"
    download_sprite(url, filename)
    time.sleep(0.1)  # Small delay to be respectful to servers

print("Download process completed!")
print(f"Sprites saved to: {output_dir}")
print("Note: Update the sprite_urls list with actual URLs from a legitimate source.")
