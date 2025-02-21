# ***`Basic Shell in C`***

[Overview](#overview) | [Features](#features) | [Installation](#installation) | [Usage](#usage) | [Project Structure](#project-structure) | [Testing](#testing) | [Known Issues](#known-issues) | [Contributing](#contributing) | [License](#license)

> [!IMPORTANT]
> This project implements a basic shell in C, capable of handling various commands, input/output redirection, and piping. The shell supports basic commands like `cd`, `echo`, and `ls`, as well as more advanced features such as command sequencing (`;`), input/output redirection (`<`, `>`), and piping (`|`).

## Overview

The Basic Shell project is a C-based implementation of a simple shell that allows users to execute commands, handle input/output redirection, and pipe commands together. It supports basic built-in commands like `cd`, `echo`, and `exit`, as well as external commands like `ls` and `mkdir`. The shell also includes a tokenizer for parsing input commands and a test suite to validate functionality.

## Features

- **Basic Commands**: Supports built-in commands like `cd`, `echo`, `help`, and `exit`.
- **Command Sequencing**: Allows multiple commands to be executed sequentially using `;`.
- **Input/Output Redirection**: Supports `<` for input redirection and `>` for output redirection.
- **Piping**: Allows the output of one command to be used as input for another using `|`.
- **Tokenizer**: Splits input strings into tokens for command parsing.
- **Testing Suite**: Includes Python-based tests to validate shell functionality.

## Installation

### Prerequisites

- **GCC (GNU Compiler Collection)**
- **Python 3** (for running tests)

### Steps

1. Clone the repository:
   ```sh
   git clone https://github.com/yourusername/basic-shell.git
   ```
2. Navigate to the project directory:
   ```sh
   cd basic-shell
   ```
3. Compile the project:
   ```sh
   make
   ```
4. Run the shell:
   ```sh
   ./shell
   ```
### Usage

#### Basic Commands

* Change Directory:
   ```sh
   cd /path/to/directory
   ```
* Print Text:
   ```sh
   echo "Hello, World!"
   ```
* List Directory Contents:
   ```sh
   ls -la
   ```
* Exit the Shell:
   ```sh
   exit
   ```
### Command Sequencing

#### Commands can be sequenced using `;`:

   ```sh
   echo "First command"; echo "Second command"
   ```

### Input/Output Redirection

* Input Redirection:
   ```sh
   sort < input.txt
   ```
* Output Redirection:
   ```sh
   echo "Hello" > output.txt
   ```

### Piping

#### Pipe the output of one command to another:

   ```sh
   ls | grep .txt
   ```
## Project Structure

```plaintext
basic-shell/
├── Makefile
├── README.md
├── examples/
│   └── tokenize_expr.c
├── shell.c
├── tests/
│   ├── shell_test_helpers.py
│   ├── shell_tests.py
│   └── tokenize_tests.py
├── tokenize.c
├── tokenize_func.c
└── tokenize_func.h
```
### Testing

#### The project includes a suite of Python-based tests to validate the shell and tokenizer functionality. To run the tests, use:

   ```sh
   make test
   ```
#### This will execute both the shell and tokenizer tests.

## Known Issues

- **Quoted Commands**: The shell may not handle quoted commands correctly in all cases, especially when combined with redirection or piping.
- **Multiple Pipes**: The shell currently has limited support for multiple pipes in a single command.
- **Command History**: The `prev` command (to repeat the last command) may not work as expected in all scenarios.

## Contribution Guidelines

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature-name`).
3. Commit changes (`git commit -m 'Add new feature'`).
4. Push to the branch (`git push origin feature-name`).
5. Open a **Pull Request**.


## License

This project is licensed under the **MIT License**. See the `LICENSE` file for details.

