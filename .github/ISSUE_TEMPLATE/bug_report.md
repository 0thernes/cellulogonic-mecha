name: Bug Report
description: Report a build failure, runtime defect, UI rendering issue, or ABI compatibility defect.
title: "[Bug]: "
labels: ["bug"]
body:
  - type: textarea
    id: summary
    attributes:
      label: Summary & Environment
      placeholder: OS, compiler (Clang/GCC), Node version, and exact command executed.
    validations:
      required: true
  - type: textarea
    id: logs
    attributes:
      label: Error Output / Logs
    validations:
      required: true
