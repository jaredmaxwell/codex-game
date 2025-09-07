# GitHub Pages Deployment Guide

This guide explains how to deploy your Codex Game to GitHub Pages, which will solve the CORS issue by serving your WebAssembly game over HTTPS.

## Why GitHub Pages?

- **Solves CORS issues**: Files are served over HTTPS, not `file://`
- **Free hosting**: No cost for public repositories
- **Automatic deployment**: Updates when you push to main branch
- **Global CDN**: Fast loading worldwide
- **Custom domain support**: Use your own domain if desired

## Setup Instructions

### 1. Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** tab
3. Scroll down to **Pages** section
4. Under **Source**, select **GitHub Actions**
5. Save the settings

### 2. Push Your Code

The deployment workflow will automatically trigger when you push to the `main` or `master` branch:

```bash
git add .
git commit -m "Add GitHub Pages deployment"
git push origin main
```

### 3. Monitor Deployment

1. Go to the **Actions** tab in your repository
2. Watch the "Deploy to GitHub Pages" workflow run
3. Once complete, your game will be available at:
   ```
   https://yourusername.github.io/your-repo-name
   ```

## Manual Deployment

If you want to deploy manually without pushing to main:

1. Go to **Actions** tab
2. Select "Deploy to GitHub Pages" workflow
3. Click **Run workflow**
4. Select the branch and click **Run workflow**

## File Structure

After deployment, your GitHub Pages site will contain:
```
/
├── game.html          # Main game file
├── game.js            # JavaScript runtime
├── game.wasm          # WebAssembly binary
└── assets/            # Game assets
    └── char.png
```

## Accessing Your Game

Once deployed, you can access your game at:
- **URL**: `https://yourusername.github.io/your-repo-name`
- **Mobile**: Works on phones and tablets
- **Desktop**: Works on all modern browsers

## Custom Domain (Optional)

To use a custom domain:

1. Add a `CNAME` file to your repository root with your domain:
   ```
   yourdomain.com
   ```

2. Configure DNS with your domain provider:
   ```
   Type: CNAME
   Name: www (or @)
   Value: yourusername.github.io
   ```

3. Enable "Enforce HTTPS" in GitHub Pages settings

## Troubleshooting

### Deployment Fails
- Check the Actions tab for error messages
- Ensure your repository is public (required for free GitHub Pages)
- Verify the workflow file is in `.github/workflows/`

### Game Doesn't Load
- Check browser console for errors
- Ensure all files (game.html, game.js, game.wasm) are present
- Try hard refresh (Ctrl+F5 or Cmd+Shift+R)

### CORS Errors Still Occur
- GitHub Pages serves over HTTPS, so CORS shouldn't be an issue
- If you see CORS errors, check that you're accessing the GitHub Pages URL, not a local file

## Benefits Over Local Server

✅ **No local setup required** - Just push code  
✅ **Always accessible** - Available 24/7  
✅ **HTTPS by default** - Secure and modern  
✅ **Global CDN** - Fast loading worldwide  
✅ **Mobile friendly** - Works on all devices  
✅ **Shareable** - Easy to share with others  

## Updating Your Game

To update your deployed game:

1. Make changes to your code
2. Commit and push to main branch:
   ```bash
   git add .
   git commit -m "Update game"
   git push origin main
   ```
3. GitHub Actions will automatically rebuild and redeploy
4. Your changes will be live in a few minutes

## Performance Tips

- **Optimize assets**: Compress images and audio files
- **Use CDN**: GitHub Pages includes global CDN
- **Monitor size**: Keep total size under 1GB (GitHub Pages limit)
- **Test on mobile**: Ensure good performance on slower connections
