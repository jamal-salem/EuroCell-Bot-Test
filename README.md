# 🛡️ EuroCell Bot Test

![Language](https://img.shields.io/badge/Language-C-blue)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6)
![Development](https://img.shields.io/badge/IDE-Visual%20Studio-purple)
![Security](https://img.shields.io/badge/Purpose-Defensive%20Security-success)
![Environment](https://img.shields.io/badge/Testing-Local%20Only-orange)

## Overview

**EuroCell Bot Test** is a controlled security-testing client written in **C** for evaluating the anti-spam protections of the EuroCell WordPress contact form in an authorized local testing environment.

The project was created to answer a practical security question:

> **Can a simple automated HTTP client submit the EuroCell contact form without successfully passing the website's anti-spam and CAPTCHA protections?**

Rather than relying only on manual browser testing, the project reproduces the basic behavior of an automated form-submission client and observes how the server responds.

The purpose of the project is **not to bypass security mechanisms**.

Its purpose is to verify that those mechanisms correctly detect and reject automated submissions.

---

## 🎯 Why This Project Exists

The EuroCell website experienced unwanted automated contact-form submissions.

Before changing security settings, installing additional protection layers, or making assumptions about the source of the spam, it was important to determine whether the existing protection mechanisms were actually capable of rejecting a basic automated client.

This project was therefore developed as a small controlled test environment for studying that behavior.

The testing methodology focuses on a simple question:

```text
Automated Client
       ↓
Contact Form
       ↓
WordPress / Everest Forms
       ↓
Anti-Spam Controls
       ↓
reCAPTCHA Validation
       ↓
Accepted or Rejected?
```

This provides a repeatable way to verify whether the current server-side protection chain is functioning as expected.

---

## 🔬 Test Environment

All automated testing is restricted to the authorized local development environment:

```text
http://eurocell-local.local
```

The production EuroCell website is **not** an automated test target.

Testing is deliberately performed using a very small number of controlled requests.

---

## ⚙️ How the Client Works

The client reproduces the minimum HTTP workflow required to perform a controlled form-submission test.

```text
GET Contact Form
       ↓
Read Current Form Data
       ↓
Extract Fresh Nonce
       ↓
Generate Submission Timestamp
       ↓
Respect Minimum Waiting Period
       ↓
Build multipart/form-data Request
       ↓
Send One Controlled POST Request
       ↓
Inspect Server Response
       ↓
Verify WordPress Form Entries
```

The program:

1. Connects to the local WordPress website.
2. Requests the page containing the Everest Forms contact form.
3. Extracts the current form nonce and required request information.
4. Generates the form submission timestamp.
5. Respects the configured minimum submission waiting period.
6. Constructs the request using `multipart/form-data`.
7. Sends one controlled automated form submission.
8. Reads and inspects the HTTP response.
9. Verifies whether the submission was accepted by WordPress.

The CAPTCHA field is intentionally submitted **without a valid CAPTCHA solution**.

This allows the test to determine whether server-side CAPTCHA validation correctly prevents a basic automated client from successfully submitting the form.

---

## ✅ Current Test Result

The controlled automated request successfully reaches the local WordPress server.

However, the form submission is rejected because a valid CAPTCHA token is not present.

The server response contains:

```text
CAPTCHA token missing. Please try again.
```

A manual verification of the WordPress form entries also confirms that the automated test submission was **not accepted**.

### Result

```text
Automated Request     → Reached Server
HTTP Communication    → Successful
Form Processing       → Triggered
CAPTCHA Validation    → Failed
Submission            → Rejected
WordPress Entry       → Not Created
```

This confirms that the existing server-side CAPTCHA validation successfully blocks this basic automated submission scenario.

---

## 🧰 Technologies Used

| Technology | Purpose |
|---|---|
| **C** | Core application language |
| **WinHTTP** | Native Windows HTTP communication |
| **Microsoft Visual Studio** | Development and debugging environment |
| **MSVC** | C compiler and build toolchain |
| **Windows** | Development platform |
| **WordPress** | Local website environment |
| **Everest Forms** | Contact-form system under test |
| **reCAPTCHA** | Server-side anti-automation protection |
| **Git** | Source-control management |
| **GitHub** | Repository hosting and project history |

---

## 📁 Project Structure

```text
EuroCell-Bot-Test/
│
├── .gitignore
├── AGENTS.md
├── EuroCell-Bot-Test.slnx
│
└── EuroCell-Bot-Test/
    ├── main.c
    ├── EuroCell-Bot-Test.vcxproj
    └── EuroCell-Bot-Test.vcxproj.filters
```

### `main.c`

Contains the C implementation of the controlled HTTP client and form-submission testing logic.

### `AGENTS.md`

Defines the project's development scope, coding guidelines, testing methodology, and security boundaries for AI-assisted development.

### Visual Studio Project Files

The `.slnx`, `.vcxproj`, and `.vcxproj.filters` files contain the Microsoft Visual Studio solution and project configuration.

---

## 🔒 Security Boundaries

This repository is intended for **authorized defensive security testing only**.

The project must not be used to:

- Bypass or solve CAPTCHA challenges.
- Use third-party CAPTCHA-solving services.
- Perform stress or load testing.
- Generate high-volume automated requests.
- Perform proxy or IP rotation.
- Modify WordPress installations without authorization.
- Attack or test third-party systems without permission.
- Use automated requests against the production EuroCell website.
- Collect credentials or other sensitive information.

The project is intentionally limited to a controlled local testing environment.

---

## 🧠 Development Methodology

The project follows a simple security-testing methodology:

```text
Observe
   ↓
Understand
   ↓
Implement
   ↓
Build
   ↓
Test
   ↓
Verify
```

### Observe

Inspect the behavior of the real browser request.

### Understand

Determine how the form actually communicates with the server, including the request method, fields, nonce, timing information, and request structure.

### Implement

Reproduce only the minimum required behavior in the C test client.

### Build

Compile and verify the application using Microsoft Visual Studio and MSVC.

### Test

Send a small number of controlled requests exclusively to the authorized local environment.

### Verify

Inspect both the HTTP response and WordPress form entries to determine whether the automated submission was accepted or rejected.

---

## 🧪 Testing Philosophy

The project follows several important principles:

**Do not guess request parameters.**

Real browser behavior should be inspected before implementing HTTP logic.

**Use the minimum number of requests necessary.**

The objective is security verification, not traffic generation.

**Verify results at multiple layers.**

An HTTP `200 OK` response alone does not prove that a form submission was accepted.

The application response and the WordPress database/form entries must also be checked.

**Keep testing local and controlled.**

Development and automated verification remain isolated from the production website.

---

## 📊 Project Status

**Current phase: Initial defensive verification completed.**

The first controlled test demonstrated that:

- The C client can communicate with the local WordPress environment.
- The application can reproduce the basic form submission workflow.
- The server receives the automated request.
- Server-side CAPTCHA validation is executed.
- A submission without a valid CAPTCHA token is rejected.
- No corresponding accepted form entry is created.

This provides a baseline for future defensive testing.

---

## 🚧 Future Development

Future development may include additional defensive analysis such as:

- Improved response diagnostics.
- Structured test-result logging.
- Additional local anti-spam verification scenarios.
- Better HTTP error reporting.
- Security-control comparison.
- Test-result documentation.
- Modularization of the HTTP and form-parsing components.

Any future testing should continue to follow the same **authorized, local-only, low-volume** security model.

---

## ⚠️ Disclaimer

This project is intended exclusively for:

- Authorized security testing
- Defensive security research
- Software development practice
- Educational purposes

Only test systems that you own or systems for which you have explicit authorization.

The repository is **not intended for CAPTCHA bypass, spam generation, unauthorized automation, or attacks against third-party systems**.

---

## 👨‍💻 Author

**Jamal Salem**

Computer Engineer  
IT Technical Support • System Diagnostics • PowerShell • Automation • Security Testing

---

## 📌 Final Note

EuroCell Bot Test demonstrates an important principle in defensive security:

> A security control should not simply be assumed to work — it should be tested, observed, and verified in a controlled environment.

The goal of this project is to turn a real-world spam problem into a structured and measurable security verification process.
