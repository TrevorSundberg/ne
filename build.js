const execa = require('execa');
const path = require('path');
const mkdirp = require('mkdirp');
const fs = require('fs');
const glob = require('glob');
const os = require('os');
const commandExists = require('command-exists');
const tempy = require('tempy');
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

function download(url, fileName)
{
  console.log(`Downloading '${url}'`);
  return new Promise((resolve) =>
  {
    const file = fs.createWriteStream(fileName);

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
  const filePath = tempy.file({
    name: fileName
  });
  await download(url, filePath);
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
    console.error(`Platform ${platform} not supported for ${info.name}`);
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

async function runClangFormat(dirs, sourceFiles)
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

async function runDoxygen(dirs)
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

async function runCmake(dirs)
{
  if (!await verifyCommand('cmake'))
  {
    return;
  }

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
    break;
  default:
    console.error('runBuild: Unhandled platform');
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
  const dirs = gatherDirectories();
  await runEslint(dirs);
  // TODO(Trevor.Sundberg): Run cmake_format.
  const sourceFiles = gatherSourceFiles(dirs);
  await runClangTidy(dirs, sourceFiles);
  await runClangFormat(dirs, sourceFiles);
  await runDoxygen(dirs);
  // TODO(Trevor.Sundberg): Run moxygen.
  await runCmake(dirs);
  await runBuild(dirs);
  // TODO(Trevor.Sundberg): Run tests.
}

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
