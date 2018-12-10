const execa = require('execa');
const path = require('path');
const mkdirp = require('mkdirp');
const fs = require('fs');
const glob = require('glob');
const os = require('os');
const commandExists = require('command-exists');
const request = require('request');

const platform = (() =>
{
  switch (os.platform())
  {
  case 'win32':
    return 'windows';
  case 'darwin':
    return 'mac';
  default:
    return 'linux';
  }
})();

function makeDir(dirPath)
{
  mkdirp.sync(dirPath, {
    recursive: true
  });
  return dirPath;
}

const dirs = (() =>
{
  // Ensuring all directories exist just makes everything easier.
  const rootDir = process.cwd();
  return {
    root: rootDir,
    packages: path.join(rootDir, 'packages'),
    tempBuild: makeDir(path.join(rootDir, 'temp_build')),
    tempDownload: makeDir(path.join(rootDir, 'temp_download')),
    tempDoxygen: makeDir(path.join(rootDir, 'temp_doxygen')),
    tempTest: makeDir(path.join(rootDir, 'temp_test')),
  };
})();

function addPath(directory)
{
  process.env.PATH += `;${directory}`;
}

function setupEnvironment()
{
  switch (platform)
  {
  case 'windows':
    addPath('C:/Program Files/LLVM/bin');
    addPath('C:/Program Files/doxygen/bin');
    addPath('C:/Program Files/CMake/bin');
    break;
  default:
    console.error(`setupEnvironment: Unhandled platform ${platform}`);
  }
}

async function checkCommand(command)
{
  try
  {
    await commandExists(command);
    return true;
  }
  catch (err)
  {
    return false;
  }
}

async function verifyCommand(command)
{
  if (!await checkCommand(command))
  {
    console.error(`command '${command}' does not exist`);
    return false;
  }
  return true;
}

function download(url, filePath)
{
  console.log(`Downloading '${url}'`);
  return new Promise((resolve) =>
  {
    const file = fs.createWriteStream(filePath);

    request(url)
      .on('error', console.error)
      .pipe(file);

    file.on('finish', () =>
    {
      file.close();
      resolve();
      console.log(`Completed '${url}'`);
    });
  });
}

async function downloadTemp(url, fileName)
{
  const filePath = path.join(dirs.tempDownload, fileName);
  if (fs.existsSync(filePath))
  {
    try
    {
      fs.unlink(filePath);
    }
    catch (err)
    {
      console.error(err);
    }
  }

  await download(url, filePath);

  // On Windows, the file is locked after it completes downloading.
  // This may be a result of a virus scanner or other process.
  for (;;)
  {
    let file = 0;
    try
    {
      file = fs.openSync(filePath, 'r');
    }
    catch (err)
    {
      continue;
    }

    fs.closeSync(file);
    break;
  }
  return filePath;
}

async function standardInstall(filePath, args)
{
  const options = {
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  await execa(filePath, args ? args : [], options);
}

async function msiInstall(filePath, extraArgs)
{
  const options = {
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  let args = ['/i', filePath, '/quiet', '/qn', '/norestart'];
  if (extraArgs)
  {
    args = args.concat(extraArgs);
  }
  await execa('msiexec', args, options);
}

async function installProgram(info)
{
  console.log(`Checking ${info.name}`);
  if (await info.check())
  {
    console.log(`Checked ${info.name}`);
    return;
  }

  const settings = info[platform];
  if (settings)
  {
    const filePath = await downloadTemp(settings.url, settings.file);
    console.log(`Installing ${info.name}`);
    await settings.install(filePath);
    console.log(`Completed ${info.name}`);
  }
  else
  {
    console.error(`installProgram: Unhandled platform ${platform} for ${info.name}`);
  }
}

function installLlvm()
{
  return installProgram({
    name: 'LLVM',
    check: async () => await checkCommand('clang-tidy') && await checkCommand('clang-format'),
    windows: {
      url: 'http://releases.llvm.org/7.0.0/LLVM-7.0.0-win64.exe',
      file: 'LLVM-7.0.0-win64.exe',
      install: filePath => standardInstall(filePath, ['/S'])
    },
  });
}

function installDoxygen()
{
  return installProgram({
    name: 'Doxygen',
    check: async () => await checkCommand('doxygen'),
    windows: {
      url: 'http://doxygen.nl/files/doxygen-1.8.14-setup.exe',
      file: 'doxygen-1.8.14-setup.exe',
      install: filePath => standardInstall(filePath, ['/VERYSILENT', '/NORESTART'])
    },
  });
}

function installCmake()
{
  return installProgram({
    name: 'CMake',
    check: async () => await checkCommand('cmake'),
    windows: {
      url: 'https://github.com/Kitware/CMake/releases/download/v3.13.1/cmake-3.13.1-win64-x64.msi',
      file: 'cmake-3.13.1-win64-x64.msi',
      install: filePath => msiInstall(filePath, ['ADD_CMAKE_TO_PATH=System'])
    },
  });
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

function gatherSourceFiles()
{
  // Gather all .c, .cpp, and .h files.
  return glob.sync('**/*.@(c|cpp|h)', {
    cwd: dirs.packages
  });
}

async function runEslint()
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

async function runClangTidy(sourceFiles)
{
  if (!await verifyCommand('clang-tidy'))
  {
    return;
  }

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
    const result = await execa('clang-tidy', ['-extra-arg=-Weverything', filePath], clangTidyOptions);
    if (result.stdout)
    {
      console.error(result.stdout);
    }
  }
}

async function runClangFormat(sourceFiles)
{
  if (!await verifyCommand('clang-format'))
  {
    return;
  }

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

async function runDoxygen()
{
  if (!await verifyCommand('doxygen'))
  {
    return;
  }

  const doxygenOptions = {
    cwd: dirs.root,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  await execa('doxygen', [], doxygenOptions);
}

async function runCmake()
{
  if (!await verifyCommand('cmake'))
  {
    return;
  }

  const cmakeOptions = {
    cwd: dirs.tempBuild,
    stdio: ['ignore', 'ignore', 'inherit'],
    reject: false
  };
  await execa('cmake', ['..'], cmakeOptions);
}

async function runBuild()
{
  const testPaths = [];
  switch (platform)
  {
  case 'windows':
    {
      // TODO(Trevor.Sundberg): We should also perform a cmake build to clang on windows.
      const vswhereOptions = {
        cwd: `${process.env['ProgramFiles(x86)']}/Microsoft Visual Studio/Installer/`,
        stdio: ['ignore', 'pipe', 'inherit'],
        reject: false
      };
      const result = await execa('vswhere.exe', vswhereOptions);
      if (!result.stdout)
      {
        console.error('No output from vswhere.exe');
        break;
      }
      const devenvPath = /productPath: (.*)/g.exec(result.stdout)[1];
      if (!devenvPath)
      {
        console.error('No "productPath" from vswhere.exe');
        break;
      }

      const logPath = path.join(dirs.tempBuild, 'output.log');
      safeDeleteFile(logPath);

      const devenvOptions = {
        cwd: dirs.tempBuild,
        stdio: ['ignore', 'ignore', 'inherit'],
        reject: false
      };
      const args = [path.join(dirs.tempBuild, 'ne.sln'), '/build', 'Debug', '/out', logPath];
      await execa(devenvPath, args, devenvOptions);
      const log = fs.readFileSync(logPath, {
        encoding: 'utf8'
      });
      safeDeleteFile(logPath);

      if (!log.match(/, 0 failed,/g))
      {
        console.error(log);
        break;
      }

      testPaths.push(path.join(dirs.tempBuild, 'Debug', 'ne.exe'));
    }
    break;
  default:
    console.error(`runBuild: Unhandled platform ${platform}`);
  }
  return testPaths;
}

async function runTests(testPaths)
{
  // TODO(Trevor.Sundberg): ne_io_input read test fails if we don't specify 'inherit' for stdin.
  for (const testPath of testPaths)
  {
    const inputPath = path.join(dirs.packages, 'test_io', 'input.txt');
    const outputPath = path.join(dirs.tempTest, 'output.txt');

    const input = fs.openSync(inputPath, 'r');
    const output = fs.openSync(outputPath, 'w');

    const options = {
      cwd: path.dirname(testPath),
      stdio: [input, output, 'inherit'],
      reject: false
    };
    await execa(testPath, ['--simulated'], options);

    fs.closeSync(input);
    fs.closeSync(output);

    const inputText = fs.readFileSync(inputPath, {
      encoding: 'utf8'
    });
    const outputText = fs.readFileSync(outputPath, {
      encoding: 'utf8'
    });

    // Due to the ne_core_hello_world, we can't expect a specific output, but
    // we know that ne_io_output tests are called after ne_core_hello_world.
    if (inputText.endsWith(outputText))
    {
      console.error(`Standard output does not match: '${outputText}'`);
    }
  }
}

async function install()
{
  await installLlvm();
  await installDoxygen();
  await installCmake();
}

async function build()
{
  await runEslint();
  // TODO(Trevor.Sundberg): Run cmake_format.
  const sourceFiles = gatherSourceFiles();
  await runClangTidy(sourceFiles);
  await runClangFormat(sourceFiles);
  // TODO(Trevor.Sundberg): Run cppcheck.
  // TODO(Trevor.Sundberg): Run cpplint.
  await runDoxygen();
  // TODO(Trevor.Sundberg): Run moxygen.
  await runCmake();
  const testPaths = await runBuild();
  await runTests(testPaths);
}

setupEnvironment();
const command = process.argv[2];
switch (command)
{
case 'install':
  install();
  break;
case 'build':
  build();
  break;
default:
  console.error(`Invalid command '${command}'`);
  break;
}
