# Assets Guide

## Directory: src/assets/icons/

This directory contains all image assets used in the application.

## Required Images

### 1. logo1.png (or logo1.jpeg)
- **Purpose**: Main application logo displayed in the sidebar
- **Recommended Size**: 70x70 pixels
- **Format**: PNG (preferred) or JPEG
- **Location in UI**: Sidebar top section (logoFrame)
- **Fallback**: If not found, a colored circle with letter "A" will be displayed

### 2. pfp.png (or pfp.jpeg)
- **Purpose**: Default profile picture for users
- **Recommended Size**: 50x50 pixels (sidebar), 32x32 pixels (navbar)
- **Format**: PNG (preferred) or JPEG
- **Location in UI**: 
  - Sidebar profile card
  - Navbar profile button
- **Fallback**: If not found, a sage green gradient circle with initials will be displayed

### 3. icon.png (or icon.jpeg)
- **Purpose**: Application window icon
- **Recommended Size**: 256x256 pixels (or multiple sizes)
- **Format**: PNG (preferred) or JPEG
- **Usage**: Window icon, taskbar icon
- **Fallback**: System default icon

## Image Formats Supported

- PNG (recommended for transparency)
- JPEG/JPG
- All images are automatically copied to the build directory

## Adding New Images

1. Place your image files in `src/assets/icons/`
2. The build system will automatically detect and copy them
3. Reference them in code using just the filename (e.g., "logo1.png")

## Code References

Images are loaded in `mainwindow.cpp`:

```cpp
// Example: Loading logo
logoLabel = createRoundedAvatar("logo1.png", 70);

// Example: Loading profile picture
QLabel *profilePhoto = createRoundedAvatar("pfp.jpeg", 32);
```

## Current Asset Structure

```
src/assets/icons/
├── logo1.png          (Your application logo)
├── pfp.jpeg           (Default profile picture)
└── icon.png           (Application icon)
```

## Build Process

CMake automatically:
1. Scans `src/assets/icons/` for all PNG/JPEG files
2. Copies them to the build directory
3. Makes them available to the application at runtime

No manual copying required!

## Tips

- Use PNG format for images with transparency (like logos)
- Use JPEG for photographs (like profile pictures)
- Keep file sizes reasonable (< 500KB per image)
- Use descriptive names for additional icons
- Test with missing images to ensure fallbacks work
