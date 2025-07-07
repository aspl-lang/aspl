# Installing ASPL
ASPL's main focus is on simplicity and ease of use. This is why the installation process is as simple as downloading and running a single installer script:

* Windows: [install.bat](https://raw.githubusercontent.com/aspl-lang/aspl/main/install.bat)
* Linux, macOS: [install.sh](https://raw.githubusercontent.com/aspl-lang/aspl/main/install.sh)

*Any operating systems not listed here (as well as all architectures other than x86_64) are not currently supported as host platforms, but may also work after some manual setup.*

The installer will install ASPL in a directory called `aspl` relative to the working directory of the script and automatically navigate into it.

> **Note:** if you run the installer inside an already checked out ASPL repository, it will automatically update the repository to the latest version.

> **Note:** you will need [`git` to be installed on your computer](https://github.com/git-guides/install-git) in order to use the ASPL installer.

> **Note:** on Unix-like systems (Linux, macOS), you may need additionally to install `jq`, `curl` and `unzip` in order to run the installer script. On Debian-based systems, you can do this by running the following command:
```sh
sudo apt install jq curl unzip
```

After you have installed ASPL, you can check if everything is working by trying to compile and run the "Hello World" program:
```sh
$ aspl run "examples/hello_world"
Hello World!
```

Congratulations! You have successfully installed ASPL!
<br>Be sure to check out the [introduction](introduction.md) next to learn more about ASPL and how to use it.

> [!TIP]
> Once you have installed ASPL on your computer, you can always easily upgrade to the latest version of it using the `aspl update` command! By doing this once in a while, you'll make sure to get access to new features, bug fixes, improved documentation, and much more without needing to manually reinstall anything.