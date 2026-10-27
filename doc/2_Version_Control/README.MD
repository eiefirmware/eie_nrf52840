# Version Control

[TOC]

## Introduction

This module introduces one of the core systems used when writing software, version control.
The most common version control software in industry is [Git](https://git-scm.com/),
which if you've already completed [Lesson 1](../1_Getting_Started/README.MD),
you've already been using!

## Motivation

How often have you found yourself:

- Wondering what you just changed in your code
- Wondering what your code looked like yesterday/last week/last time you worked on it
- Copying your code somewhere else to save a copy as-is
  - Maybe to try a new approach that may or may not work
  - Maybe to have a copy of what your code was at certain milestones/submissions/etc
  - Maybe because you just solved a bug, and want to make sure that you always have a working version without that bug saved "just in case"
- Struggling to combine the work of several people working on a single project at once
- Struggling to keep your code in-sync between different computers/people

Chances are, if you write code, you've probably already run into some of these.
If you haven't, and _don't_ use version control, you almost certainly will.

A program like Git effectively allows you to control and re-write time when it comes to your projects.
By creating "commits", you can save snapshots of what your project looked like at different times.
Then whenever you want,
you can "checkout" these commits and instantly have your project's contents become what they were when the commit was created.
You can then upload your project (or "repository") to a remote service like [GitHub](https://github.com/eiefirmware).
This allows you to "push" your local changes to an internet copy of your project,
and then "pull" them down on another computer.
This is even more useful when working in groups,
as all of your work gets stored and managed from a single, universal location,
but everyone is still able to do anything they want to the files on their own computer.

## Lesson

The goal of this lesson is to become familiar enough with Git and GitHub to use them effectively throughout EiE.
To do so, we're going to go through a set of lessons and exercises:

1. Go to ["Learn Git Branching"](https://learngitbranching.js.org/)
   1. Complete levels 1-4 of the "Introductory Sequence" section under the "Main" tab
   2. Complete levels 1-7 of the "Push & Pull -- Git Remotes!" section under the "Remote" tab
   3. NOTE: You can use the `levels` command to open up the level selector menu
2. With that primer, lets create a Git command cheatsheet for you to reference in your repository!
   1. Open up your `zephyr-projects/eie_nrf52840.git` in Git Bash
      1. Remember, your `zephyr-projects` directory is where you created it in [Lesson 1](../1_Getting_Started/README.MD)
      2. Some useful Bash commands if you're not yet super comfortable in a terminal:
         1. `cd <dir>` - "change directory" - Bash command to move to the given `<dir>` directory
         2. `pwd` - "print working directory" - Bash command to check which directory the terminal is currently in
         3. `ls` - "list" - Bash command to show contents of the current directory (files beginning with `.` are hidden, like `.git` or `.vscode`)
         4. `ls -a` - "list all" - Bash command to show _all_ contents of the current directory
   2. use `git remote -v` to check that your 'origin' is set to your forked repo and that 'upstream' is set to the eie repo
      a. It should look like this
         ```
         origin git@github.com:<username>/eie_nrf52840.git (fetch)
         origin git@github.com:<username>/eie_nrf52840.git (push)
         upstream git@github.com:eiefirmware/eie_nrf52840.git (fetch)
         upstream git@github.com:eiefirmware/eie_nrf52840.git (push)
         ```
      b. If you don't see the `upstream` options, run `git remote add upstream git@github.com:eiefirmware/eie_nrf52840.git` and check again
   3. Run `git fetch upstream main` then `git pull upstream main`, then finally `git push origin main` to update your fork
   4. Ensure you're on the `main` branch by running `git branch`
      1. In Git Bash, this is also shown in blue text in your terminal when you're inside of a Git repository (you should be!)
      2. If the output or blue text is something other than `main`, you've already made another branch! Return to the `main` branch by running `git checkout main`
   5. Ensure your local Git repo is up to date with `git pull`
   6. Create a new branch for this lesson with `git branch lesson2`
   7. Move to your new branch with `git checkout lesson2`
   8. Make a file in the `doc/2_Version_Control/` directory called `git_cheatsheet.txt`
      1. You can open vscode at this location by typing `code .`
      2. Using vscode, you can create `git_cheatsheet.txt` and open it to edit
      3. Fill this file with a list of all the Git commands you know so far
         1. `git commit`
         2. `git branch`
         3. `git checkout`
         4. `git pull`
         5. `git push`
         6. `git merge`
         7. `git rebase`
         8. `git fetch`
      4. Write a brief description of what the command does, and what arguments you know that you can pass to it
         1. Does the function of the command change depending on the number of arguments? At least one of these commands does!
         2. Don't worry about finding every possible argument the command can take, there are a HUGE amount, most of which you won't need for EiE. Just focus on what you know so far!
   9. Check that you see that a new file has been added in Git Bash by running `git status`
      1. You should see a new "untracked file" in red. This is because the Git repo hasn't been told to track this file yet, so let's do that.
   10. Add this file to your "staging area" with `git add doc/2_Version_Control/git_cheatsheet.txt`
   11. See that the file you've added has been "staged" for a commit by running `git status` again
       1. You should see an "added file" in green. Any and all "staged" (green) changes are added to a commit when one is made, none of the "unstaged" changes (red).
   12. Commit your new cheatsheet to your repo with `git commit -m 'Add Git Cheatsheet'`
       1. The `-m` stands for "message", and allows for you to pass a "commit message" to the command as the next argument (done in single quotes here)
       2. You can instead just run `git commit`, which will open up a text editor for you to write longer commit messages
   13. Check where your commit is in the Git history "tree" by running `git log`
       1. You should see your commit at the very top, with a `HEAD -> lesson2` label, meaning that your repo's "HEAD" is pointing to the `lesson2` branch, which itself is on your new commit
       2. Newest commit is shown at the top, with older commits shown below
       3. **If your terminal shows a colon in the bottom left and isn't letting you type commands as usual, press `q` to exit back to a normal terminal session**
   14. Check that you no longer see your new file as a "change" in `git status`
   15. "Push" your new `lesson2` branch and its commit to your fork of the EiE project repo on GitHub with `git push -u origin lesson2`
       1. You will be prompted for/asked to set your GitHub username and password if you haven't pushed to GitHub before
       2. The `-u origin lesson2` is only needed because this is the _first_ time that we're pushing the `lesson2` branch to GitHub (it tells GitHub to make this new branch on the remote). After this, you should only run `git push`.
          1. **If you run into an issue where Git asks for your password but fails with an error saying "Password authentication is disabled", see the "Personal Access Tokens" section below.**
   16. Check on GitHub that **your fork** of the `eie_nrf52840` repo has a new `lesson2` branch, and that your cheatsheet is on it
3. Congratulations! You've now completed an initial Git workflow!
4. You've now learned a few new Git commands, why not add them to your cheatsheet?
   1. Staying on your `lesson2` branch, update your `git_cheatsheet.txt` to include:
      1. `git add`
      2. `git status`
      3. `git log`
      4. A note about `-u <remote> <branch>` under `git push`
   2. Once the changes have been added, check what's changed with `git diff`
      1. Might as well add that to the cheatsheet too!
      2. **If your terminal shows a colon in the bottom left and isn't letting you type commands as usual, press `q` to exit back to a normal terminal session**
   3. Make a new commit once this is done, checking what you're doing with `git status` and `git log` as before
   4. Push this commit to the your fork on GitHub with `git push` again
   5. Once again, check on GitHub that your `lesson2` branch has been updated
5. If you complete all of the above, do one or more of the following:
   1. Play around with some Git commands in your local repo. Try changing/making more branches, commits, etc.
   2. Complete more of the lessons on ["Learn Git Branching"](https://learngitbranching.js.org/)

As you complete more lessons in EiE,
add what you've learned about Git and how to use it to this cheetsheat,
it will always be safe on this branch!

## Git Workflow for Future Lessons

In general, a good Git process to follow for EiE is as follows:

1. Checkout the `main` branch
2. Pull any remote changes to your local repo to ensure that the `main` branch is up-to-date
3. Create a `lesson#` branch (e.g. `lesson2`)
4. Do all work for that lesson on that branch
   1. Create commits at points where you've achieved a goal/milestone, completed a "substantial" amount of work or other similar marker
   2. Gauging commit size is a balancing act that is only learned with experience. Use EiE as a safe environment to experiment!
5. At the end of the lesson, push your `lesson#` branch to GitHub

Of course, feel free to experiment with different workflows, but always be prepeared to explain what you've done to anyone you're expecting help from!

## Source Control and VSCode

VSCode has several tools for working with Git.
As mentioned in Lesson 1, you can access the source control view in the activity bar or via `CTRL+SHIFT+G` (`⌘+SHIFT+G` on mac).
When you open the source control view you will see something similar to the view below where you see files you have changed at the top and the Git tree at the bottom

![Source Control View](imgs/source_control.png)

In the changes view it shows you all files that have been modified since your last commit.
You can use the plus button next to a file to stage a file, and the back arrow to revert your change.
If you click on any file it will open a view showing what has changed.
Finally you can use the Message bar at the top to write your commit message.

While the default source control activity view has a source graph, the Git Graph extension is often more useful.
You can start it using the `Git Graph` button in the primary side bar of the status bar in VSCode.
![Git Graph Button](imgs/git_graph_button.png)

This will open a view similar to below where you can click on any commit to see its full commit message, and the files changed.
![Git Graph](imgs/git_graph.png)

For a more in depth view as to what these features can do read the
- [VSCode Source Control Documentation](https://code.visualstudio.com/docs/sourcecontrol/overview)
- [Git Graph Documentation](https://github.com/mhutchie/vscode-git-graph/wiki)

## Additional Resources

Below are additional resources for learning more about Git

* [Git learning videos](https://git-scm.com/videos)
* [Git Book](https://git-scm.com/book/en/v2)
* [Git Cheatsheet](https://git-scm.com/cheat-sheet)
