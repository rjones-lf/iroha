# Contributing guidelines

:star::tada: First off, thanks for taking the time to contribute! :tada::star:

The following is a short set of guidelines for contributing to Iroha. 



#### Table Of Contents

##### [How Can I Contribute?](#how-can-i-contribute-1)

- [Reporting bugs](#reporting-bugs)
- [Suggesting Enhancements](#suggesting-enhancements)
- [Asking Questions](#asking-questions)
- [Your First Code Contribution](#your-first-code-contribution)
- [Pull Requests](#pull-requests)

##### [Styleguides](#styleguides-1)

- [Git Commit Messages](#git-commit-messages)
- [C++ StyleGuide](#C++-styleguide)
- [Documentation Styleguide](#documentation-styleguide)

##### [Additional Notes](#additional-notes)

- [Informational Labels](#informational-labels)
- [Pull Request and Issue Labels](#pull-request-and-issue-labels)
- [Issue Labels](#issue-labels)
- [Pull Request Labels](#pull-request-labels)
- [Contact Developers](#contact-developers)



## How Can I Contribute?

### Reporting bugs

*Bug* is an error, design flaw, failure or fault in Iroha that causes it to produce an incorrect or unexpected result, or to behave in unintended ways. 

Bugs are tracked as [GitHub Issues](https://guides.github.com/features/issues/). To submit a bug, create new Issue and include these details:
- **Title**
    - Write prefix `[Bug]` for the title
    - Use a clear and descriptive title
- **Body** - include the following sections:
    - System environment (OS, iroha version)
    - Steps to reproduce
    - Expected behavior
    - Actual behavior



### Suggesting Enhancements

An *enhancement* is a code or idea, which makes **existing** code or design faster, more stable, portable, secure or better in any other way.

Enhancements are tracked as [GitHub Issues](https://guides.github.com/features/issues/). To submit new enhancement, create new Issue and incllude these details:

- **Title**
    - Write prefix `[Enhancement]`
    - Use a clear and descriptive title
- **Body** - include the following sections:
    - *Target* - what is going to be improved?
    - *Motivation* - why do we need it?
    - *Description* - how to implement it?



### Asking Questions

A *question* is any discussion that is typically neigher a bug, nor feature request, nor improvement - "How do I do X?".

Questions are tracked as [Github Issues](https://guides.github.com/features/issues/) or via private messages in [your favourite messenger](#contact-developers).

To submit new question in GitHub Issues, it must include these details:

- **Title**
    - Write prefix `[Question]`
    - Use a clear and descriptive title
- **Body** - describe your question with as many details as possible.



### Your First Code Contribution

Read our [C++ Style Guide](#c++-style-guide) and start with beginner-friendly issues with label [`[good-first-issue]`](https://github.com/hyperledger/iroha/issues?q=is:open+is:issue+label:good-first-issue ). Indicate somehow that you are working on this task.



### Pull Requests

- Fill in [the required template](.github/PULL_REQUEST_TEMPLATE.md)

- **Write tests** for new code. Test coverage for new code must be at least 70%.

- Every pull request should be reviewed and **get at least two approvals**.

- Do not include issue numbers in the PR title or commit messages.

- Use [keywords for closing issues](https://help.github.com/articles/closing-issues-using-keywords/).

- Include issue numbers in Pull Request body only.

- When finished work, **rebase onto base branch** with 
    ```bash
    $ git fetch
    $ git rebase -i <base-branch>
    ```

    [Step-by-step guide](https://soramitsu.atlassian.net/wiki/spaces/IS/pages/11173889/Rebase+and+merge+guide).

- Follow the [C++ Style Guide](#C++-style-guide).

- Follow the [Git Style Guide](#git-commit-messages) .

- **Document new code** based on the [Documentation Styleguide](#documentation-styleguide)

- End all files with a newline.




## Styleguides

### Git Style Guide

- **Use present tense** ("Add feature", not "Added feature").
- **Use imperative mood** ("Deploy docker to..." not "Deploys docker to...").
- Write meaningful commit message.
- **Signed-off every commit** with [DCO](https://github.com/apps/dco): `Signed-off-by: $NAME <$EMAIL>`. 
    You can do it automatically using `git commit -s`.
- Do not include PR or Issue number in commit message. 
- Limit the first line of commit message to 50 characters or less.
- First line of commit message must contain summary of work done, second line must contain empty line, third and other lines can contain list of commit changes.
- When only changing documentation, include `[ci skip]` in the commit description.
- We use mixed approach of [Github Flow](https://guides.github.com/introduction/flow/) and [Git Flow](http://nvie.com/posts/a-successful-git-branching-model/). More at [Iroha Working Agreement](https://github.com/hyperledger/iroha/wiki/Iroha-working-agreement#2-version-control-system).




### C++ Style Guide

- Use [clang-format](http://clang.llvm.org/docs/ClangFormat.html) for code formatting (we use google code style). 
- Follow [CppCoreGuidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) and [Cpp Best Practices](https://lefticus.gitbooks.io/cpp-best-practices).
- Avoid [platform-dependent](https://stackoverflow.com/questions/1558194/learning-and-cross-platform-development-c) code.
- Use [C++14](https://en.wikipedia.org/wiki/C%2B%2B14).
- Use [camelCase](https://en.wikipedia.org/wiki/Camel_case) for class names and methods, use [snake_case](https://en.wikipedia.org/wiki/Snake_case) for variables.



### Documentation Styleguide

- Use [Doxygen](http://www.stack.nl/~dimitri/doxygen/manual/docblocks.html).
- Document all public API: methods, functions, members, templates, classes...


## Additional Notes

### Informational Labels
| Label Name              | Description                                                      |
| :---------------------- | ---------------------------------------------------------------- |
| `pri:low`               | Low priority.                                                    |
| `pri:normal`            | Normal priority.                                                 |
| `pri:important`         | Important issue.                                                 |
| `pri:critical`          | Critical issue. Must be fixed immediately.                       |
| `pri:blocker`           | Issue blocked by other issues.                                   |
| `status:in-progress`    | Work in progress.                                                |
| `status:inactive`       | Inactive PR or Issue. Likely to become a `candidate-for-closing` |
| `status:wontfix`        | Core team has decided not to fix these issue for now.            |

### Issue and Pull Request labels

| Label Name              | Description                                                  |
| :---------------------- | ------------------------------------------------------------ |
| `enhancement:code`      | Any improvements in **existing** code.                       |
| `enhancement:idea`      | Fresh ideas to enhance existing architecture, design.        |
| `bug:needs-reproduction`| Bugs or reports that are very likely to be bugs.             |
| `bug:confirmed`         | Confirmed bug by maintainers.                                |
| `feature`               | Feature requests -- completely new functionality.            |
| `accepted`              | Pull request is accepted and can be merged.                  |
| `candidate-for-closing` | Outdated Pull Request / Issue. Lasts for more than 14 days.  |
| `needs-correction`      | Pull Request / Issue that should be corrected by author.     |
| `needs-review`          | Pull Request / Issue that should be reviewed by maintainer.  |


### Issue Labels

| Label Name              | Description                                                           |
| :---------------------- | --------------------------------------------------------------------- |
| `question`              | Questions more than bug reports or feature requests - "How do I do X" |
| `good-first-issue`      | Good starting point to begin contributing.                            |
| `help-wanted`           | Maintainers ask for help to work on this issue.                       |

### Pull Request Labels

| Label Name              | Description                                 |
| :---------------------- | ------------------------------------------- |
| `accepted`              | Pull request is accepted and can be merged. |


### Contact Developers

Developers are available at:

| Service      | Link                                     |
| ------------ | ---------------------------------------- |
| RocketChat   | https://chat.hyperledger.org/channel/iroha |
| Mailing List | [hyperledger-iroha@lists.hyperledger.org](mailto:hyperledger-iroha@lists.hyperledger.org) |
| Gitter       | https://gitter.im/hyperledger-iroha/Lobby |
| Telegram     | https://t.me/joinchat/Al-9jkCZ6eePL9JMQtoOJw |



---

Thank you for reading the document! 
