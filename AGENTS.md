# Repository Guidelines

## Project Purpose

EuroCell-Bot-Test is a small C program used for controlled security verification of the local EuroCell WordPress contact form.

The goal is to determine whether a simple automated HTTP client is rejected by the existing anti-spam controls.

## Project Scope

Work only inside this repository.

Primary source:

`EuroCell-Bot-Test/main.c`

The project is built with Microsoft Visual Studio and MSVC.

Use C source files (`.c`) unless there is a documented reason to introduce another language.

## Safety Rules

Target only:

`http://eurocell-local.local`

Never send automated requests to:

`https://eurocelltsc.com`

Do not:

- bypass or solve reCAPTCHA
- use CAPTCHA-solving services
- perform stress or load testing
- generate high request volumes
- use proxy or IP rotation
- modify the WordPress installation
- guess the Everest Forms submission endpoint or field names
- expose credentials, secret keys, or private tokens

Use only a small number of controlled requests.

## Development Workflow

Before implementing HTTP submission logic, inspect the real successful Everest Forms browser request captured from Chrome DevTools.

Determine the actual:

- request URL
- HTTP method
- payload fields
- form ID
- nonce/token fields
- cookie requirements
- reCAPTCHA token representation

Do not invent missing request details.

## Coding Style

Use clear ISO C-compatible code.

Use four spaces for indentation and descriptive names.

Keep functions small and focused.

Check return values from networking and system functions.

Do not add unnecessary dependencies.

## Build and Verification

Build the Debug x64 configuration in Visual Studio before testing.

Expected workflow:

`Capture → Understand → Implement → Build → Send one local request → Inspect response → Verify Everest Forms Entries`

A bot test passes only when the automated submission is rejected and no accepted Everest Forms entry is created.

## Agent Behavior

Explain significant changes before making them.

Prefer minimal changes over broad refactoring.

Do not alter unrelated files.

When information is missing, stop and request evidence instead of guessing.