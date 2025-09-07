# Quick Fix: GitHub Pages Setup

## The Error
```
Error: Get Pages site failed. Please verify that the repository has Pages enabled and configured to build using GitHub Actions
```

This happens because GitHub Pages isn't enabled in your repository settings yet.

## Solution

### Option 1: Enable Pages Manually (Recommended)

1. **Go to your repository on GitHub**
2. **Click the "Settings" tab** (at the top of your repo)
3. **Scroll down to "Pages"** in the left sidebar
4. **Under "Source"**, select **"GitHub Actions"**
5. **Click "Save"**

### Option 2: Let the Workflow Enable It

I've updated the workflow to automatically enable Pages. Just push the updated workflow:

```bash
git add .github/workflows/deploy-pages.yml
git commit -m "Fix GitHub Pages workflow with auto-enablement"
git push origin main
```

## After Enabling Pages

Once Pages is enabled, your workflow will:
1. ✅ Build your WebAssembly game
2. ✅ Deploy it to GitHub Pages
3. ✅ Make it available at `https://yourusername.github.io/codex-game`

## Verify It's Working

1. **Check the Actions tab** - The workflow should run successfully
2. **Go to Settings → Pages** - You should see "Your site is live at..."
3. **Visit your game** - It will be at the URL shown in Pages settings

## Troubleshooting

### Still Getting Errors?
- Make sure your repository is **public** (required for free GitHub Pages)
- Check that you have **write permissions** to the repository
- Verify the workflow file is in `.github/workflows/deploy-pages.yml`

### Pages Still Not Working?
- Try the manual setup (Option 1) first
- Wait a few minutes after enabling - GitHub needs time to set up
- Check the Actions tab for any error messages

## Your Game URL

Once working, your game will be available at:
```
https://yourusername.github.io/codex-game
```

Replace `yourusername` and `codex-game` with your actual GitHub username and repository name.
