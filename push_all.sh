#!/bin/bash

USER="sashwithnathreddy-ctrl"

push_repo() {
    REPO_PATH=$1
    REPO_NAME=$2
    echo "=============================="
    echo "Pushing $REPO_NAME from $REPO_PATH"
    echo "=============================="
    cd "$REPO_PATH" || { echo "Path not found: $REPO_PATH"; return; }
    git init
    git add .
    git commit -m "Upload all files" || echo "Nothing to commit"
    git branch -M main
    git remote remove origin 2>/dev/null
    git remote add origin "https://github.com/$USER/$REPO_NAME.git"
    git push -u origin main || {
        echo "Push rejected, trying pull --rebase..."
        git pull --rebase origin main
        git push -u origin main
    }
    echo ""
}

push_repo "$HOME/skilling" "skilling"
push_repo "$HOME/skilling/OS_Pract2" "OS_Pract2"
push_repo "$HOME/ShellForge" "ShellForge"

echo "✅ All repositories pushed!"
