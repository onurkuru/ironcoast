# Self-contained Mac playtest delivery

1. [NEW] tools/package_macos.py: assemble app with assets/licenses and its SDL library, rewrite only the copied binary dependency, verify no Homebrew dependency remains, ad-hoc sign. Derive minimum macOS from the executable build metadata.
2. Validate the packaged executable from a different working directory with automatic bundled asset discovery; run the existing application smoke cases against it.
3. [MODIFY] docs/DEVELOPMENT_STATUS.md, system_architecture.md, task.md: document Apple Silicon/local signing limits and packaging commands.
4. Update local launcher and GitHub. Preserve existing player.save and old playtest files.
