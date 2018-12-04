const execa = require('execa');
const path = require('path');
const mkdirp = require('mkdirp');
const fs = require('fs');
const glob = require('glob');
const os = require('os');

function gatherDirectories()
{
  const rootDir = process.cwd();
  return {
    root: rootDir,
    packages: path.join(rootDir, 'packages'),
    build: path.join(rootDir, 'build'),
  };
}

function safeDeleteFile(filePath)
{
  try
  {
    fs.unlinkSync(filePath);
  }
  catch (err)
  {
    // Ignored.
  }
}

function gatherSourceFiles(dirs)
{
  // Gather all .c, .cpp, and .h files.
  return glob.sync('**/*.@(c|cpp|h)', {
    cwd: dirs.packages
  });
}

async function runEslint(dirs)
{
  const eslintOptions = {
    cwd: dirs.root,
    stdio: ['ignore', 'pipe', 'inherit'],
    reject: false
  };
  const result = await execa('node', ['node_modules/eslint/bin/eslint.js', '.'], eslintOptions);
  if (result.stdout)
  {
    console.error(result.stdout);
  }
}

async function runClangTidy(dirs, sourceFiles)
{
  // Run clang-tidy.
  const clangTidyOptions = {
    cwd: dirs.packages,
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
}

async function runClangFormat(dirs, sourceFiles)
{
  const clangFormatOptions = {
    cwd: dirs.packages,
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

    const fullPath = path.join(dirs.packages, filePath);
    const oldCode = fs.readFileSync(fullPath, {
      encoding: 'utf8'
    });
    const newCode = result.stdout;

    if (oldCode !== newCode)
    {
      console.error(`File '${fullPath}' was not formatted`);
    }
  }
}

async function runDoxygen(dirs)
{
  const doxygenOptions = {
    cwd: dirs.root,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  await execa('doxygen', [], doxygenOptions);
}

async function runCmake(dirs)
{
  const cmakeOptions = {
    cwd: dirs.build,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  mkdirp.sync(dirs.build, {
    recursive: true
  });
  await execa('cmake', ['..'], cmakeOptions);
}

async function runBuild(dirs)
{
  if (os.platform() === 'win32')
  {
    const vswhereOptions = {
      cwd: `${process.env['ProgramFiles(x86)']}/Microsoft Visual Studio/Installer/`,
      stdio: ['ignore', 'pipe', 'inherit'],
      reject: false
    };
    const result = await execa('vswhere.exe', vswhereOptions);
    if (!result.stdout)
    {
      console.error('No output from vswhere.exe');
      return;
    }
    const devenvPath = /productPath: (.*)/g.exec(result.stdout)[1];
    if (!devenvPath)
    {
      console.error('No "productPath" from vswhere.exe');
      return;
    }

    const logPath = path.join(dirs.build, 'output.log');
    safeDeleteFile(logPath);

    const devenvOptions = {
      cwd: dirs.build,
      stdio: ['ignore', 'ignore', 'inherit'],
      reject: false
    };
    const args = [path.join(dirs.build, 'ne.sln'), '/build', 'Debug', '/out', logPath];
    await execa(devenvPath, args, devenvOptions);
    const log = fs.readFileSync(logPath, {
      encoding: 'utf8'
    });
    safeDeleteFile(logPath);

    if (!log.match(/, 0 failed,/g))
    {
      console.error(log);
    }
  }
  else
  {
    console.error('Unhandled platform');
  }
}

async function main()
{
  const dirs = gatherDirectories();
  await runEslint(dirs);
  const sourceFiles = gatherSourceFiles(dirs);
  await runClangTidy(dirs, sourceFiles);
  await runClangFormat(dirs, sourceFiles);
  await runDoxygen(dirs);
  // Run moxygen.
  await runCmake(dirs);
  await runBuild(dirs);
  // Run tests.
}

main();
