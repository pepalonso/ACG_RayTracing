# GitHub Actions - Automated Rendering

This repository includes a GitHub Actions workflow that automatically builds and runs the ray tracer when you push to the main branch.

## How It Works

1. **Automatic Trigger**: Runs on every push to `main` or `master` branch
2. **Manual Trigger**: Can also be triggered manually from GitHub's Actions tab
3. **Build Process**: 
   - Sets up Ubuntu with CMake and C++ compiler
   - Builds the project in Release mode
   - Runs the ray tracer executable
4. **Output**: Both `output.bmp` and `output.exr` are saved as downloadable artifacts

## How to Get Your Rendered Images

### After Pushing to GitHub:

1. Go to your repository on GitHub
2. Click the **"Actions"** tab at the top
3. Click on the most recent workflow run
4. Scroll down to the **"Artifacts"** section
5. Download:
   - `rendered-output-bmp` - The BMP image file
   - `rendered-output-exr` - The EXR file (HDR format)

### Manual Triggering:

1. Go to **Actions** tab
2. Click **"Render Ray Tracing Scene"** in the left sidebar
3. Click **"Run workflow"** button (top right)
4. Select branch and click **"Run workflow"**
5. Wait for completion and download artifacts

## Workflow Details

- **Runtime**: ~2-5 minutes (build + render time)
- **Cost**: Free (uses GitHub's free tier)
- **Retention**: Artifacts kept for 90 days
- **Platform**: Ubuntu latest (Linux)

## Troubleshooting

If the workflow fails:
- Check the workflow logs in the Actions tab
- Ensure all source files compile locally first
- Check that CMakeLists.txt is correctly configured

## Customization

To modify what gets rendered, edit `src/main.cpp` and commit/push.
The workflow will automatically pick up your changes!

