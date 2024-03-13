% Hacking

We're thrilled to have you here! Your interest in making olang the most
exceptional and straightforward language ever is greatly appreciated.

In this document, we'll outline how you can begin contributing to olang.

First and foremost, clone the project if you haven't done so already.

``` {.sh}
git clone https://git.sr.ht/~johnnyrichard/olang
```

Dependencies
------------

The olang compiler is crafted in C. To build the project, you'll require
merely three dependencies: **make**, **gcc** (version 11 or higher), and
**clang-format** (version 14 or higher).

As an optional step, you can install **sphinx** to refresh this
documentation.

Code style
----------

Instead of delineating every element of our coding style, we have
adopted the use of **clang-format** to enforce the olang code style.
Please refer to the linter section below for guidance on its
application.

### Linter

Checking for linter issues:

``` {.sh}
make linter
```

Most of the common code style mistakes are fixed by:

``` {.sh}
make linter-fix
```

### .editorconfig

We also provide a **.editorconfig** file at project\'s root. Follow
<https://editorconfig.org/> to learn how to make it work with your
favorite editor.

Testing
-------

There are two layers of tests **integration** and **unit**. The
integration test is going to execute the **olang** compiler and check if
the generated binary acts as expected. Unit tests will test C functions.

For both unit and integration we use **munit** framework:
<https://nemequ.github.io/munit/>.

To execute tests you can execute:

``` {.sh}
make check
```

Submitting a patch
------------------

All contributors are required to "sign-off" their commits (using git commit -s)
to indicate that they have agreed to the [Developer Certificate of
Origin](https://developercertificate.org/).

Before submit the patch, ensure the code follows our coding style and is
well-tested. After that, you\'re good to follow the steps bellow.

### Step 1: Commit your changes

Begin by committing your modifications with a detailed and significant
commit message. We take great pleasure in maintaining a record of our
changes over time. Skeptical? Execute a **git log** command and admire
the well-documented history we've created so far.

But it isn\'t all about personal preference. We use a email-driven
workflow to propose our changes, meaning the commit message you write is
the email the olang maintainers will get. I won't go into the perks of
the email-driven workflow here, but you can check it out at
<https://drewdevault.com/2018/07/02/Email-driven-git.html>.

#### Best practices

1. Write single-purpose commits.
2. Write a meaningful commit message.
3. Every commit must be production ready.
    - If the tests or the linter fail, you should not create a fix commit.
      Instead, you should amend the commit that caused the issue and then
      resend the patchset.

### Step 2: Create your patch

You can create a patch using the command:

``` {.sh}
git format-patch --cover-letter -M origin/main -o outgoing/
```

### Step 3: Write a cover letter:

The command above generates a **outgoing/0000-cover-letter.patch** file.

The cover letter is like a pull request description on Github. Replace
the \*\*\* SUBJECT HERE \*\*\* with the patchset title and the
\*\*\* BLURB HERE \*\*\* with a synopsis of your changes.

If you are sending a single-commit patch you can remove the
**\--cover-letter** argument from **git format-patch** and skip this
step.

### Step 4: Submit your patch

Make sure you have configured your **git send-email**. You can learn how
to configure it here:
<https://git-scm.com/docs/git-send-email#_examples>

Once you have everything set you just need to send the patch over our
mailing list.

``` {.sh}
git send-email outgoing/* --to=~johnnyrichard/olang-devel@lists.sr.ht
```

### The review process

Upon submission, you'll receive an automated email from our pipeline. If
the check is successful, the olang maintainers will review your patch.
Subsequently, you'll receive an email indicating whether your patch has
been approved, requires changes, or has been rejected.

### Submitting changes in a patchset

If your patchset requires any modifications, you'll have to submit a new
version of your patch. The submission process remains unchanged, except
for the addition of the version argument to the **git format-patch**
command.

``` {.sh}
git format-patch --cover-letter -M origin/main -o outgoing/ -v2
```

After send a new email with **git send-email**.

------------------------------------------------------------------------

Thanks again and happy hacking!
