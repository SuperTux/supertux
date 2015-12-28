# Contributing to SuperTux

## Unsure? [Contact Us](http://supertuxproject.org/contact.html)

## Bug reports

- GitHub Issues in English only. IRC support might be possible in other languages,
  however English is generally preferred so everyone can understand what's happening.
- Please check for duplicate issues on GitHub, as they're wasting the developers'
  time.
- Also try to check if your issue has been resolved in the development version.
- Separate issues and feature requests. Don't put all in the same issue.


### Required information

Please include the following details in your issue report so that we can investigate:

- The SuperTux version you're using, as printed in the bottom left corner of the
  menu or in `supertux2 -v`.
- Details about your kernel, operating system and distribution, including name
  and version details.
- If possible, provide steps to reproduce your issue.
- If available, it's a great help for the developers to include a backtrace from
  gdb or a core dump. If you're on GNU/Linux and your distro is using systemd, it's
  highly likely that a core dump is available using `coredumpctl(1)`.

### Add-on related issues

No support is provided for third-party add-ons (that doesn't include official
add-ons or language packs). However, a pull request against the SuperTux/addons
repository containing a fix is always welcome.

### Translation issues

Please use Transifex to fix or report translation related issues.

## Pull requests

- No duplicate pull requests, please.
- Please test your code to see if it works correctly. If the pull request
  is untested or work-in-progress, please mention that.
- Pull requests importing translation updates from Transifex are, unless requested
  by a contributor, not very welcome.
- Commit messages should follow this style: short summary up to 50 chars (72 are
  okay if you need that much), an empty line, more detailed explanatory text which
  can include multiple paragraphs or bullet points. 
- If necessary, `Closes #xyz.` where #xyz is the number of the issue which that pull request will fix.
- If your commit doesn't include any changes that need CI testing (no code changes e.g. graphics), 
  add `[ci skip]` to any line which isn't the first or second. This saves IRC spam.
- Please [squash](https://help.github.com/articles/about-git-rebase/) commits that belong together. 
  If you forget initially, it's okay to force push to your own fork to fix it.
- No merge commits inside pull requests. Use `git-rebase` to keep your branch
  up-to-date.
- To make it easier for everyone, don't make a pull request pulling your fork's
  `master` branch.
