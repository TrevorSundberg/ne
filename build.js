const execa = require('execa');
const path = require('path');
const mkdirp = require('mkdirp');
const fs = require('fs');
//const file = require('file'); // Remove me if unused
const glob = require('glob');

async function main()
{
  const stdioErrorOnly = ['ignore', 'ignore', 'inherit'];

  const rootDir = process.cwd();
  const rootOptions = {
    cwd: rootDir,
    stdio: stdioErrorOnly,
    reject: false
  };

  await execa('node', ['node_modules/eslint/bin/eslint.js', '.'], rootOptions);

  const packagesDir = path.join(rootDir, 'packages');

  const sourceFiles = glob.sync('**/*.@(c|h|cpp)', {
    cwd: packagesDir
  });

  // Run clang-tidy.
  for (const filePath of sourceFiles)
  {
    // Clang-tidy emits all the errors to the standard out.
    // We capture them and re-emit them to stderr.
    const result = await execa('clang-tidy', [filePath], {
      cwd: packagesDir,
      stdio: ['ignore', 'pipe', 'ignore'],
      reject: false
    });

    if (result.stdout)
    {
      console.error(result.stdout);
    }
  }

  // Run clang-format.
  for (const filePath of sourceFiles)
  {
    // Clang-format emits the formatted file to the output.
    // In this build script we want to emit errors if the user
    // did not auto-format their code, so we will perform a diff.
    const result = await execa('clang-format', [filePath], {
      cwd: packagesDir,
      stdio: ['ignore', 'pipe', 'ignore'],
      reject: false,
      stripFinalNewline: false,
      stripEof: false
    });

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

  const buildDir = path.join(rootDir, 'build');
  const buildOptions = {
    cwd: buildDir,
    stdio: stdioErrorOnly,
    reject: false
  };
  mkdirp.sync(buildDir, {
    recursive: true
  });
  await execa('cmake', ['..'], buildOptions);

}

main();

// Run tests
// Run doxygen
// Run moxygen
