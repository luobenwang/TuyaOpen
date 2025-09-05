#!/usr/bin/env python3
"""
PNG to RGB565 Converter
Convert PNG images to RGB565 format C arrays for embedded systems
"""

import os
import sys
from PIL import Image
import argparse

def rgb888_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    r = (r >> 3) & 0x1F
    g = (g >> 2) & 0x3F
    b = (b >> 3) & 0x1F
    return (r << 11) | (g << 5) | b

def png_to_rgb565_array(png_path, output_path, array_name):
    """Convert PNG image to RGB565 C array"""
    try:
        # Open and convert image
        img = Image.open(png_path)
        
        # Convert to RGB if necessary
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        width, height = img.size
        pixels = list(img.getdata())
        
        # Convert to RGB565
        rgb565_data = []
        for r, g, b in pixels:
            rgb565 = rgb888_to_rgb565(r, g, b)
            rgb565_data.append(rgb565)
        
        # Generate C array
        with open(output_path, 'w') as f:
            f.write(f"// Auto-generated from {os.path.basename(png_path)}\n")
            f.write(f"// Size: {width}x{height} pixels\n")
            f.write(f"// Format: RGB565\n\n")
            f.write(f"#include <stdint.h>\n\n")
            f.write(f"const uint16_t {array_name}_width = {width};\n")
            f.write(f"const uint16_t {array_name}_height = {height};\n")
            f.write(f"const uint16_t {array_name}_data[] = {{\n")
            
            # Write data in rows of 16
            for i in range(0, len(rgb565_data), 16):
                row = rgb565_data[i:i+16]
                hex_values = [f"0x{val:04X}" for val in row]
                f.write(f"    {', '.join(hex_values)},\n")
            
            f.write("};\n")
        
        print(f"Successfully converted {png_path} to {output_path}")
        print(f"Image size: {width}x{height}, Data size: {len(rgb565_data) * 2} bytes")
        return True
        
    except Exception as e:
        print(f"Error converting {png_path}: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Convert PNG images to RGB565 C arrays')
    parser.add_argument('input_dir', help='Input directory containing PNG files')
    parser.add_argument('output_dir', help='Output directory for C files')
    parser.add_argument('--emotion-map', help='Emotion mapping file (optional)')
    
    args = parser.parse_args()
    
    # Create output directory if it doesn't exist
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Emotion mapping
    emotion_map = {
        'angry': 'angry',
        'happy': 'happy', 
        'love': 'love',
        'sad': 'sad',
        'shock': 'shock',
        'sleep': 'sleep',
        'think': 'think'
    }
    
    # Load custom emotion mapping if provided
    if args.emotion_map and os.path.exists(args.emotion_map):
        with open(args.emotion_map, 'r') as f:
            for line in f:
                if '=' in line:
                    key, value = line.strip().split('=', 1)
                    emotion_map[key.strip()] = value.strip()
    
    success_count = 0
    total_count = 0
    
    # Process all PNG files
    for filename in os.listdir(args.input_dir):
        if filename.lower().endswith('.png'):
            png_path = os.path.join(args.input_dir, filename)
            base_name = os.path.splitext(filename)[0]
            
            # Get emotion name from mapping
            emotion_name = emotion_map.get(base_name, base_name)
            array_name = f"emotion_{emotion_name}_image"
            
            output_path = os.path.join(args.output_dir, f"{array_name}.c")
            
            if png_to_rgb565_array(png_path, output_path, array_name):
                success_count += 1
            total_count += 1
    
    print(f"\nConversion complete: {success_count}/{total_count} files converted successfully")
    
    # Generate header file
    header_path = os.path.join(args.output_dir, "emotion_images.h")
    with open(header_path, 'w') as f:
        f.write("#ifndef EMOTION_IMAGES_H\n")
        f.write("#define EMOTION_IMAGES_H\n\n")
        f.write("#include <stdint.h>\n\n")
        
        for emotion_name in emotion_map.values():
            array_name = f"emotion_{emotion_name}_image"
            f.write(f"extern const uint16_t {array_name}_width;\n")
            f.write(f"extern const uint16_t {array_name}_height;\n")
            f.write(f"extern const uint16_t {array_name}_data[];\n\n")
        
        f.write("#endif // EMOTION_IMAGES_H\n")
    
    print(f"Header file generated: {header_path}")

if __name__ == "__main__":
    main() 