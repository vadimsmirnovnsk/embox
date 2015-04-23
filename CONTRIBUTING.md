Contributing to Embox
=====================

Thank you for taking the time to make Embox better! This makes you awesome by
default, but there are some rules to adhere, and following this guide will help
you make the whole thing about 20% cooler.

The guide describes **a workflow accepted in the project**. Experienced Git
users will likely be happy with a "TL; DR" excerpt just below. For the rest of
folks there is a more explanatory manual going after that covering the workflow
step-by-step.

There are also several **[checklists](#checklists)** to check with (including
commit message agreements), and as a bonus, a **[cheatsheet](#cheatsheet)** on
fixing some common problems and rewriting the history.

Git workflow
------------
We use [GitHub flow](https://guides.github.com/introduction/flow/) for the
development, which is basically about committing into branches and merging
through pull requests. **TL; DR**:
  - Git setup: real name and email; Editors/IDE: proper indentation, auto strip
    [trailing whitespaces](http://codeimpossible.com/2012/04/02/Trailing-whitespace-is-evil-Don-t-commit-evil-into-your-repo-/)
  - Never push to `master`; start each new feature in a branch
  - Branch naming: *`short-name-through-hyphens`*; use *`NNNN-bug-name`* to
    refer an issue
  - Within a branch, please keep the history linear, i.e. `rebase` instead of
    `merge` to keep the branch up-to-date
  - [Atomic](http://www.freshconsulting.com/atomic-commits/) commits;
    no "oops" commits, [squash](#squash-commits-into-a-single-one) if needed
  - 50/72 commit message agreements;
    [the Seven Rules](http://chris.beams.io/posts/git-commit/#seven-rules)
    plus:
    - *`subsystem: module: Use colons for topic / scope`*, and
    - *`subsystem: (minor) Put labels in parens after the topic`*
  - Also please consult the [checklists](#checklists) below

Before you start
----------------
Make sure to perform the necessary
[setup steps](https://help.github.com/articles/set-up-git/):

```sh
git config --global user.name "YOUR NAME"
git config --global user.email "YOUR EMAIL ADDRESS"
```

Please use your real name (no nicknames, sorry).

There is also a recommended setup, please see [below](#recommended-setup).

Clone the repository
----------------------
If you're new to GitHub's Fork & Pull model, you'll probably find
this [help article](https://help.github.com/articles/fork-a-repo/) useful.

In a nutshell to clone your fork and setup the upstream:

```sh
git clone https://github.com/YOUR-USERNAME/embox embox
cd embox

git remote add upstream https://github.com/embox/embox.git

git remote -v
# origin    https://github.com/YOUR-USERNAME/embox.git (fetch)
# origin    https://github.com/YOUR-USERNAME/embox.git (push)
# upstream  https://github.com/embox/embox.git (fetch)
# upstream  https://github.com/embox/embox.git (push)
```

Contributors with push access clone the original repository directly, i.e.
the upstream is `origin`:

```sh
git clone https://github.com/embox/embox embox
cd embox

# (optionally) Setup upstream as an alias of origin
git remote add upstream https://github.com/embox/embox.git
```

Note: Depending on whether you have
[SSH keys](https://help.github.com/articles/generating-ssh-keys/) set up,
you may wish to prefer using `git@github.com:embox/embox.git` as the clone URL.

Feature process
---------------
Do these things for each feature you work on. To switch features, just use `git
checkout my-branch`.

### Start a new feature branch

Create a new branch from `master`. The code in `master` always
compiles and passes all tests (well, it tends to).

    git fetch upstream
    git checkout upstream/master -b my-branch

  - Give a new branch some descriptive name, separate words with hyphens

  - If it's a feature branch, it may be worth spending some time to open an
    issue and describe your intentions. Someone may already have had some
    thoughts on the topic and could probably give you some advice

  - It is a good practice to begin a branch name with a number of the issue
    being addressed, like *`NNNN-bug-name`*, for example,
    *`721-contributing-guide`*. This will update the issue status accordingly

### Do the work

*Hack, hack, hack...*

### Synchronize with the upstream

Before publishing your work please pick up all recent upstream changes. May be
someone has added a fix for a bug that you were forced to work around?

The main point here is to `rebase`, i.e. "replay" your changes on top of the
updated `upstream/master`.

  - In case you're on a fork, to keep it up-to-date, please, follow
    [this guide](https://help.github.com/articles/syncing-a-fork/), except for
    a `merge` part

  - When doing `pull`, always do it with `--rebase` to avoid unnecessary merge
    commits:

        git checkout my-branch
        git pull --rebase upstream

    - Setup `git config --bool pull.rebase true` to make `pull` imply
      `--rebase` by default
    - You might prefer using tools like
      [git-up](http://aanandprasad.com/git-up/)

  - To catch up any new changes from `master` use `rebase` instead of `merge`:

        git checkout my-branch
        git fetch upstream
        git rebase upstream/master

### Push the changes

Please spend extra 5-15 minutes to organize your commits. The person who will
review your changes would really appreciate that.

  - Use `git commit --amend` to edit the last commit, don't produce
    unnecessary "oops" commits

  - Befriend `git rebase --interactive`

  - Quite often, especially when developing something from scratch,
    you'll only need a single commit: add a new driver, fix a crash, etc.

Please also refer to the [checklists](#checklists) and the
[cheatsheet](#cheatsheet) below.

Then just do it:

    git push origin my-branch

To push a new branch created locally, you will need to set up tracking
information:

    git push --set-upstream origin my-branch

If you edit the history (to integrate feedback, or to add a change you forgot
to commit to an existing commit), you will need to use `--force` to push the
branch. When force-pushing, **always specify the branch explicitly**:

    git push --force origin my-branch

  - Also setup `git config push.default simple` for extra safety

> Force-pushing is safe provided two things are true:
>   - The branch has not yet been merged to the upstream repo
>   - You are only force-pushing to your fork, not to the upstream repo
>
> Generally, no other users will have work based on your pull request, so
> force-pushing history won't cause problems.

### Open a pull request

Before opening a new pull request for you branch, especially for long running
branches, please take a look at you changes once again. Basically, it's worth
performing the same checks as for [pushing](#pushing-the-changes) described
above.

It is also worth [synchronizing](#synchronize-with-the-upstream) your local
repository with the upstream to catch up all recent updates from
`upstream/master` and checking once again that nothing is broken. This way, it
is guaranteed that your branch will merge cleanly, i.e. the pull request can be
merged by the "Merge" button from the GitHub web interface.

### Integrating feedback

Very likely, you will be advised on how to make your proposed changes better.
It is OK to add follow-up changes as separate commits especially during the
review process.

However, it is a good practice to squash the follow-up changes into the
appropriate commits to make the history look like as if you made everything
correctly right from the beginning. And moreover this is the strict requirement
for follow-up changes like fixing whitespace errors or bad code formatting.

As already mentioned above, it is likely that your feature requires only a
single commit. [See below](#squash-commits-into-a-single-one) on how to combine
commits into a single one.

Checklists
----------
### Publishing a branch

Before proposing a new Pull Request, please check that:
  - [x] None of the commits introduce whitespace errors
    - Rebase with `--whitespace=fix` to fix them, if any; see the
      [cheatsheet](#fix-whitespace-errors)

  - [x] There are no merge commits, e.g. you didn't accidentally `pull` without
    `--rebase`

  - [x] There are no "oops" or fixup commits; don't forget to squash them

  - (ideally) The whole set of commits is bisectable, i.e. the code
    builds and runs fine after applying each commit one by one

### Committing best practices

For each commit, besides the above, please also be sure that:
  - [x] The commit is [atomic](http://www.freshconsulting.com/atomic-commits/):
    - Commit each fix or task as a separate change
    - Only commit when a block of work is complete (or commit and squash
      afterwards)
    - Layer changes, e.g. first make a separate commit formatting the code,
      then commit the actual logic change. This way, the latter commit contains
      the only minimal diff, which is to be reviewed

  - [x] That is, the commit isn't mixing:
    - ... whitespace changes or code formatting with functional code changes
    - ... two unrelated functional changes

  - [x] The commit log message follows the rules
    [below](#commit-message-agreements)

### Commit message agreements
Based on the excellent
[How to write a Git Commit Message](http://chris.beams.io/posts/git-commit/)
guide:

  - **Separate subject from body with a blank line**
  - **Start the subject with a topic / subsystem**, e.g.
    _**`util: list: `**`Add an unsafe version of ...`_
  - When specifying labels, put them after the topic, i.e.
    _`util: `**`(docs) (minor) `**`Fix a typo in ...`_
  - Capitalize the sentence after the topic / labels
    - Except for words that must be lowercased, like a module / source name
  - Do not end the subject line with a period
    - Except for ending a sentence with common abbreviations, *e.g. etc.*
  - **Use the imperative mood in the subject line.** *`Fix`*, not
    *`fixing/fixed`*
  - Keep the subject concise: 50-60 characters as a rule of thumb;
    **the hard limit is 72**
  - **Wrap the body at 72 characters**:
    [stopwritingramblingcommitmessages.com](http://stopwritingramblingcommitmessages.com)
    - Except for code blocks, long URLs, references to other commits: paste
      these verbatim, but indent with 4 spaces and surround with empty lines

Each of the above help to keep the history clean and consistent and require
zero effort to follow once you get used to. Most of them are enforced by
[Commitsan](https://github.com/abusalimov/commitsan).

Besides that, please pay extra attention to the following:

  - The first commit line — the subject — is the most important.
    Make the subject **as specific and definite as possible**

    - The subject appears on the project feed, in the output of
      `git log --oneline`, in the tree view of `gitk`, etc.

    - This is what a person sees when looking through a history in order
      to track changes to a certain topic

    - Summarize **what is actually changed**, not just the effect of that
      change.

      Compare:

          |     |
      --- | ---
      *fs: Add missing 'foo -> bar' dependence* | *Fix build*
      *arm/stm32: (template) Decrease thread pool size to fit RAM* | *Fix arm/stm32 template*
      *util: (refactor) Extract foo_check_xxx() from foo_func()* | *util: Work on foo refactoring*

  - Use the body to **explain *what and why*, not _how_**:

    > In most cases, you can leave out details about how a change has been
    > made. Code is generally self-explanatory in this regard (and if the code
    > is so complex that it needs to be explained in prose, that's what source
    > comments are for). Just focus on making clear the reasons you made the
    > change in the first place — the way things worked before the change (and
    > what was wrong with that), the way they work now, and why you decided to
    > solve it the way you did.
    >
    > The future maintainer that thanks you may be yourself!

  - Whenever possible, please include references to issues or links to
    discussions on the mailing list:
    - `#123` to refer an issue; `owner/repo#234` - an issue of an arbitrary
      repository
    - Use [closing keywords](https://help.github.com/articles/closing-issues-via-commit-messages/):
      *`This fixes #765 and closes #700`*; this way, once your branch gets
      merged the mentioned issues will close automatically.

To put everything together:

    subsystem: Capitalized, short summary (ideally 50 chars or less)

    More detailed description, if necessary. It should be wrapped to 72
    characters. The blank line separating the summary from the body is
    critical (unless you omit the body entirely); various tools like
    `log`, `shortlog` and `rebase` can get confused if you run the two
    together.

    The description section can have multiple paragraphs separated with
    blank lines.

    Try to be as descriptive as you can. Even if you think that the
    commit content is obvious, it may not be obvious to others.

    Code examples can be embedded by indenting them with 4 spaces:

        /* This way, lines longer than 72 characters won't trigger an error.
         * As a bonus, this makes the whole commit message more readable. */
        int main(int argc, char **argv) {
            printf("Hello world!\n");
            return 0;
        }

    You can also add bullet points:

    - Typically a hyphen (-) or an asterisk (*) is used for the
      bullet point, with blank lines in between

    - As always, wrap lines at 72 characters, and indent any
      additional lines with 2 spaces for better readability

    Provide references to issue tracker, if any.

    See also: #741 "README: Add Getting Started guide"
    Closes #721 "New README and contributing guides"

Cheatsheet
----------

There is a bunch of typical scenarios when you prepare your changes for the
review. We tried to put these together among with commands necessary to make
things done. Most of them are about *editing* the history by `git rebase` and
its derivatives.

Be sure to run the following commands after synchronizing your local clone.

    git fetch upstream

  - Or, to fetch *and* update local branches to be able to use `master` instead
    of `upstream/master` in the commands below:

        git pull --rebase upstream

It is also assumed that you're on your branch:

    git checkout my-branch

### Linearize the branch

`git rebase` is designed to "replay" a set of commits on top of a new "base"
commit.

The following command replays your branch on top of `master`. As a bonus,
it removes merge commits, if any.

Affects the whole branch, including already published changes, if any:

    git rebase upstream/master

### Fix whitespace errors

On your local commits only:

    git rebase --whitespace=fix

  - This only works if the branch is tracking its remote counterpart

On the whole branch, including already published changes, if any:

    git rebase --whitespace=fix upstream/master

  - Also use for newly created local branches without remote tracking
    information

### Undo the last commit

This is what `git reset` for.

Bring you back to the exact state just before you've done `git commit`:

    git reset --soft HEAD^

  - This preserves the current working copy, as well as the stage

The same as the above, but also resets the stage, i.e. you'll need to `git add`
from the beginning:

    git reset HEAD^

However, it is often an overkill to use `git reset`. There are more convenient
commands in case you just want to edit a commit instead of throwing it away
completely.

### Amend the last commit

It sometimes happens, that you just miss something in your last commit: forgot
to add a line to some file, made a typo, need to "fix a bugfix", etc.

Often, there's no need to `reset` the whole commit and to redo it from scratch.

`git commit` has a handy `--amend` option for that that instead of creating a
new "oops" commit, add the changes to the last one pointed by `HEAD`:

    git commit --amend

  - This can also be used to fix the commit log message

### Amend a commit buried in the history

There are two options for doing that.

#### `edit` rebase and `commit --amend`

Since `git commit --amend` allows one to only edit *the last* commit, to amend
an arbitrary commit you "rewind" the history, stop at the commit you wish to
edit, amend it, and "resume".

This is basically what `git rebase --interactive` for:

    git rebase --interactive

Or, to include already published changes:

    git rebase --interactive upstream/master

This opens an editor with a "todo" list of yet unpublished commits. Find the
commit you wish to edit and change *`pick`* in front of it to *`edit`*. Save
and close the editor. You will be advised what to do next.

You can amend the commit now, with:

    git commit --amend

Once you are satisfied with your changes, run:

    git rebase --continue

In case you just want to edit the commit log message, there's more "light"
option for that: instead of *`edit`* use *`reword`*

#### `commit --fixup` followed by `rebase --autosquash`

If you don't want to interrupt your workflow to rebase, there's yet another
option.

Once you realize that some previous commit need amendment, you create a
so-called "fixup" commit with:

```sh
git commit --fixup "SHA"
```

Which basically only creates a commit with a special message starting with
*`fixup!`* followed by a subject of the target commit:

    fixup! A subject of the commit to fix

That is, you just create a *temporary* "oops" commit. But instead of naming it
like *`(oops) Forgot to add a foo.h file`* you write *`fixup! net: Add foo
driver`*. You may create multiple such commits and then run interactive
rebasing with an `--autosquash` flag:

    git rebase --interactive --autosquash

This will prepare a "todo" list with your fixup commits put where appropriate
and the action set to *`fixup`*. Just review the list, save and close the
editor.

Note: this may sometimes result in conflicts since a fixup commit is created on
top of a snapshot that may diverge from the target commit too much.

### Squash commits into a single one

If it happen that you made some "oops" or fixup commits, you may be asked to
combine them into a single atomic commit. To achieve this, use interactive
rebasing as described above:

    git rebase --interactive

And, to include already published changes:

    git rebase --interactive upstream/master

In the "todo" list put *`squash`* in front of commits that need to be merged
into the proper one:

    pick   cf5e2f4 net: Add support for ...
    squash b54da44 (oops) Missing header file
    squash acd54ef (minor) Fix code style
    squash fe29a8c sorry, remove trailing whitespaces

Once you save and close the editor, you'll be prompted to review the final
commit log message of the combined commit. Remove unnecessary lines and save.

    Successfully rebased and updated refs/heads/my-branch

Hint: you might also use *`fixup`* instead of *`squash`*. This would do the
same, but won't ask to review the commit message.

Recommended setup
-----------------

TBD

    git config --global push.default simple
    git config --global --bool pull.rebase true
    git config --global --bool rebase.autosquash true

Some more useful config options with explanation can be found
[here](http://grimoire.ca/git/config).
