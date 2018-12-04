const execa = require('execa');
const path = require('path');
const mkdirp = require('mkdirp');
const fs = require('fs');
//const file = require('file'); // Remove me if unused
const glob = require('glob');

async function main()
{
  const rootDir = process.cwd();

  // Run eslint.
  const eslintOptions = {
    cwd: rootDir,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  await execa('node', ['node_modules/eslint/bin/eslint.js', '.'], eslintOptions);

  // Gather all .c, .cpp, and .h files.
  const packagesDir = path.join(rootDir, 'packages');
  const sourceFiles = glob.sync('**/*.@(c|cpp|h)', {
    cwd: packagesDir
  });

  // Run clang-tidy.
  const clangTidyOptions = {
    cwd: packagesDir,
    stdio: ['ignore', 'pipe', 'ignore'],
    reject: false
  };
  for (const filePath of sourceFiles)
  {
    // Clang-tidy emits all the errors to the standard out.
    // We capture them and re-emit them to stderr.
    const result = await execa('clang-tidy', [filePath], clangTidyOptions);
    if (result.stdout)
    {
      console.error(result.stdout);
    }
  }

  // Run clang-format.
  const clangFormatOptions = {
    cwd: packagesDir,
    stdio: ['ignore', 'pipe', 'ignore'],
    reject: false,
    stripFinalNewline: false,
    stripEof: false
  };
  for (const filePath of sourceFiles)
  {
    // Clang-format emits the formatted file to the output.
    // In this build script we want to emit errors if the user
    // did not auto-format their code, so we will perform a diff.
    const result = await execa('clang-format', [filePath], clangFormatOptions);

    const fullPath = path.join(packagesDir, filePath);
    const oldCode = fs.readFileSync(fullPath, {
      encoding: 'utf8'
    });
    const newCode = result.stdout;

    if (oldCode !== newCode)
    {
      console.error(`File '${fullPath}' was not formatted`);
    }
  }

  // Run doxygen.
  const doxygenOptions = {
    cwd: rootDir,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  await execa('doxygen', [], doxygenOptions);

  // Run cmake.
  const buildDir = path.join(rootDir, 'build');
  const cmakeOptions = {
    cwd: buildDir,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  mkdirp.sync(buildDir, {
    recursive: true
  });
  await execa('cmake', ['..'], cmakeOptions);

}

main();

// Run tests
// Run doxygen
// Run moxygen
