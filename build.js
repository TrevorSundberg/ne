const execa = require('execa');
const path = require('path');
const mkdirp = require('mkdirp');
const fs = require('fs');
const glob = require('glob');
const os = require('os');
const request = require('request');
const rimraf = require('rimraf');
const mv = require('mv');
const ncp = require('ncp');
const { performance } = require('perf_hooks');
const extract = require('extract-zip');
const archiver = require('archiver');

const bytesPerMb = 1024 * 1024;

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

const executableExtension = (() =>
{
  switch (os.platform())
  {
  case 'win32':
    return '.exe';
  default:
    return '';
  }
})();

const dirs = (() =>
{
  const root = process.cwd();
  const packages = path.join(root, 'packages');
  const temp = path.join(root, 'temp');
  const tempBuild = path.join(temp, 'build');
  const tempDownload = path.join(temp, 'download');
  const tempDoxygen = path.join(temp, 'doxygen');
  const tempTest = path.join(temp, 'test');
  const tempTools = path.join(temp, 'tools');
  const tempImages = path.join(temp, 'images');
  const localLlvm = path.join(tempTools, 'llvm');
  const localDoxygen = path.join(tempTools, 'doxygen');
  const localCmake = path.join(tempTools, 'cmake');
  const localLlvmBin = path.join(localLlvm, 'bin');
  const localDoxygenBin = path.join(localDoxygen, 'bin');
  const localCmakeBin = path.join(localCmake, 'bin');

  return {
    root,
    temp,
    packages,
    tempBuild,
    tempDownload,
    tempDoxygen,
    tempTest,
    tempTools,
    tempImages,
    localLlvm,
    localDoxygen,
    localCmake,
    localLlvmBin,
    localDoxygenBin,
    localCmakeBin,
  };
})();

const paths = (() =>
{
  const clang = path.join(dirs.localLlvm, 'bin', 'clang' + executableExtension);
  const clangTidy = path.join(dirs.localLlvm, 'bin', 'clang-tidy' + executableExtension);
  const clangFormat = path.join(dirs.localLlvm, 'bin', 'clang-format' + executableExtension);
  const doxygen = path.join(dirs.localDoxygen, 'bin', 'doxygen' + executableExtension);
  const cmake = path.join(dirs.localCmake, 'bin', 'cmake' + executableExtension);

  return {
    clang,
    clangTidy,
    clangFormat,
    doxygen,
    cmake,
  };
})();

function makeDir(dirPath)
{
  mkdirp.sync(dirPath, {
    recursive: true
  });
  return dirPath;
}

function makeAllDirs()
{
  // Ensuring all directories exist just makes everything easier.
  for (const key in dirs)
  {
    makeDir(dirs[key]);
  }
}

function addPath(directory)
{
  process.env.PATH += `;${directory}`;
}

function sleep(ms)
{
  return new Promise(resolve =>
  {
    setTimeout(resolve, ms);
  });
}

function printIndented(text, printer, symbol)
{
  if (!text)
  {
    return;
  }

  if (text.stack)
  {
    text = `${text.stack}`;
  }
  else
  {
    text = `${text}`;
  }

  const indent = ' '.repeat(8) + symbol;
  const matches = text.match(/[^\r\n]+/g);
  if (matches)
  {
    for (const line of matches)
    {
      printer(indent + line);
    }
  }
  else if (text)
  {
    printer(indent + text);
  }
}

function printError(text)
{
  printIndented(text, console.error, '- ');
}

function printLog(text)
{
  printIndented(text, console.log, '+ ');
}

async function exec(executable, args, options)
{
  // A file may be locked after it completes downloading.
  // This may be a result of a virus scanner or other process.
  let lastErr = 'Unknown';
  let result = null;
  for (let attempts = 10; attempts >= 0; --attempts)
  {
    try
    {
      result = await execa(executable, args, options);
      break;
    }
    catch (err)
    {
      lastErr = err;
      await sleep(100);
      continue;
    }
  }

  if (result)
  {
    if (options.err)
    {
      options.err(result.stderr);
    }
    if (options.out)
    {
      options.out(result.stdout);
    }
  }
  else
  {
    printError(lastErr);
  }
  return result;
}

async function setupEnvironment()
{
  switch (platform)
  {
  case 'windows':
    addPath('C:/Program Files/LLVM/bin');
    addPath('C:/Program Files/doxygen/bin');
    addPath('C:/Program Files/CMake/bin');
    break;
  case 'linux':
    //addPath(path.join(dirs.tempTools, 'llvm'));
    //addPath(path.join(dirs.tempTools, 'doxygen'));
    //addPath(path.join(dirs.tempTools, 'cmake'));
    break;
  default:
    printError(`setupEnvironment: Unhandled platform ${platform}`);
  }
}

function verifyFileExists(filePath)
{
  if (!fs.existsSync(filePath))
  {
    printError(`file '${filePath}' does not exist`);
    return false;
  }
  return true;
}

function getPercent(count, total)
{
  return total !== 0 ? Math.round((count / total) * 100) : '?';
}

function download(url, filePath)
{
  console.log(`Downloading '${url}'`);
  return new Promise(resolve =>
  {
    const file = fs.createWriteStream(filePath);

    let expectedBytes = 0;
    let downloadedBytes = 0;
    let lastBytes = 0;
    let lastTime = performance.now();
    request(url)
      .on('error', printError)
      .on('response', data =>
      {
        expectedBytes = data.headers['content-length'];
      })
      .on('data', chunk =>
      {
        downloadedBytes += chunk.length;
        const now = performance.now();
        if (now - lastTime >= 1000)
        {
          const deltaSeconds = (now - lastTime) / 1000.0;
          const deltaMb = (downloadedBytes - lastBytes) / bytesPerMb;

          lastTime = now;
          lastBytes = downloadedBytes;

          const mb = Math.round(downloadedBytes / bytesPerMb);
          const mbps = deltaMb / deltaSeconds;
          const percent = getPercent(downloadedBytes, expectedBytes);

          console.log(` - ${percent}% - ${mb}mb (${mbps} MB/s)`);
        }
      })
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
  try
  {
    fs.unlinkSync(filePath);
  }
  catch (err)
  {
    // Ignored.
  }

  await download(url, filePath);

  return filePath;
}

async function standardInstall(filePath, args)
{
  const options = {
    stdio: ['ignore', 'pipe', 'pipe'],
    out: printLog,
    err: printError,
    reject: false
  };
  await exec(filePath, args ? args : [], options);
}

function getSingleSubDirectory(parentDir)
{
  let singleDirectory = null;
  let directories = 0;
  let files = 0;
  for (const name of fs.readdirSync(parentDir))
  {
    const checkPath = path.join(parentDir, name);
    if (fs.lstatSync(checkPath).isDirectory())
    {
      singleDirectory = checkPath;
      ++directories;
    }
    else
    {
      ++files;
    }
  }

  if (directories === 1 && files === 0)
  {
    return singleDirectory;
  }
  return null;
}

async function tarExtractor(filePath, tempExtractDir)
{
  const options = {
    stdio: ['ignore', 'pipe', 'pipe'],
    out: printLog,
    err: printError,
    reject: false
  };
  const args = ['xf', filePath, '-C', tempExtractDir];
  return await exec('tar', args, options);
}

async function zipExtractor(filePath, tempExtractDir)
{
  const options = {
    dir: tempExtractDir
  };
  return await new Promise(resolve =>
  {
    extract(filePath, options, err =>
    {
      printError(err);
      resolve();
    });
  });
}

async function extractInstall(extractor, filePath, extractDir)
{
  const tempExtractDir = path.join(dirs.tempDownload, 'extract');
  mkdirp.sync(tempExtractDir);

  await extractor(filePath, tempExtractDir);

  // Check if there is a single directory. If so we're going to move it.
  const singleSubDir = getSingleSubDirectory(tempExtractDir);
  const moveDir = singleSubDir ? singleSubDir : tempExtractDir;

  rimraf.sync(extractDir);

  const mvOptions = {
    mkdirp: true
  };
  const error = await new Promise(resolve => mv(moveDir, extractDir, mvOptions, resolve));
  printError(error);

  rimraf.sync(tempExtractDir);
}

async function tarInstall(filePath, extractDir)
{
  return await extractInstall(tarExtractor, filePath, extractDir);
}

async function zipInstall(filePath, extractDir)
{
  return await extractInstall(zipExtractor, filePath, extractDir);
}

async function installProgram(info)
{
  if (info.check)
  {
    console.log(`Checking ${info.name}`);
    if (await info.check())
    {
      console.log(`Checked ${info.name}`);
      return;
    }
  }

  const settings = info[platform];
  if (settings)
  {
    let filePath = null;
    if (settings.url && settings.file)
    {
      filePath = await downloadTemp(settings.url, settings.file);
    }

    console.log(`Installing ${info.name}`);
    await settings.install(filePath);
    console.log(`Completed ${info.name}`);
  }
  else
  {
    printError(`installProgram: Unhandled platform ${platform} for ${info.name}`);
  }

  if (info.check)
  {
    if (await info.check())
    {
      console.log(`Checked ${info.name}`);
    }
    else
    {
      printError(`Failed ${info.name}`);
    }
  }
}

function installLlvm()
{
  return installProgram({
    name: 'Llvm',
    check: async () => fs.existsSync(paths.clang) && fs.existsSync(paths.clangTidy) && fs.existsSync(paths.clangFormat),
    windows: {
      url: 'http://releases.llvm.org/7.0.0/LLVM-7.0.0-win64.exe',
      file: 'LLVM-7.0.0-win64.exe',
      install: async filePath =>
      {
        // Do a standard install, then copy the artifacts to our local directory.
        const installPath = 'C:\\Program Files\\LLVM';
        await standardInstall(filePath, ['/S', '/D=' + installPath]);
        await new Promise(resolve =>
        {
          ncp(installPath, dirs.localLlvm, err =>
          {
            printError(err);
            resolve();
          });
        });
      }
    },
    linux: {
      url: 'http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz',
      file: 'clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz',
      install: filePath => tarInstall(filePath, dirs.localLlvm)
    },
  });
}

function installDoxygen()
{
  return installProgram({
    name: 'Doxygen',
    check: async () => fs.existsSync(paths.doxygen),
    windows: {
      url: 'http://doxygen.nl/files/doxygen-1.8.15.windows.x64.bin.zip',
      file: 'doxygen-1.8.15.windows.x64.bin.zip',
      install: filePath => zipInstall(filePath, dirs.localDoxygenBin)
    },
    linux: {
      url: 'http://doxygen.nl/files/doxygen-1.8.15.linux.bin.tar.gz',
      file: 'doxygen-1.8.15.linux.bin.tar.gz',
      install: filePath => tarInstall(filePath, dirs.localDoxygen)
    },
  });
}

function installCmake()
{
  return installProgram({
    name: 'CMake',
    check: async () => fs.existsSync(paths.cmake),
    windows: {
      //url: 'https://github.com/Kitware/CMake/releases/download/v3.13.2/cmake-3.13.2-win64-x64.zip',
      url: 'https://github.com/TrevorSundberg/ne_files/releases/download/v0.0/cmake-3.13.2-win64-x64.zip',
      file: 'cmake-3.13.2-win64-x64.zip',
      install: filePath => zipInstall(filePath, dirs.localCmake)
    },
    linux: {
      //url: 'https://github.com/Kitware/CMake/releases/download/v3.13.2/cmake-3.13.2-Linux-x86_64.tar.gz',
      url: 'https://github.com/TrevorSundberg/ne_files/releases/download/v0.0/cmake-3.13.2-Linux-x86_64.tar.gz',
      file: 'cmake-3.13.2-Linux-x86_64.tar.gz',
      install: filePath => tarInstall(filePath, dirs.localCmake)
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
  // Gather all .c, .cpp, .h, and .hpp files.
  return glob.sync('**/*.@(c|cpp|h|hpp)', {
    cwd: dirs.packages
  });
}

async function runEslint()
{
  console.log('Running Eslint');
  const eslintOptions = {
    cwd: dirs.root,
    stdio: ['ignore', 'pipe', 'pipe'],
    out: printError,
    err: printError,
    reject: false
  };
  await exec('node', ['node_modules/eslint/bin/eslint.js', '.'], eslintOptions);
}

async function runClangTidy(sourceFiles)
{
  console.log('Running Clang Tidy');
  if (!await verifyFileExists(paths.clangTidy))
  {
    return;
  }

  // Run clang-tidy.
  const clangTidyOptions = {
    cwd: dirs.packages,
    // We only ignore stderr because it prints 'unable to find compile_commands.json'.
    stdio: ['ignore', 'pipe', 'ignore'],
    out: printError,
    reject: false
  };
  for (const filePath of sourceFiles)
  {
    // Clang-tidy emits all the errors to the standard out.
    // We capture them and re-emit them to stderr.
    await exec(paths.clangTidy, ['-extra-arg=-Weverything', '-header-filter=.*', filePath], clangTidyOptions);
  }
}

async function runClangFormat(sourceFiles)
{
  console.log('Running Clang Format');
  if (!await verifyFileExists(paths.clangFormat))
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
    const result = await exec(paths.clangFormat, [filePath], clangFormatOptions);

    const fullPath = path.join(dirs.packages, filePath);
    const oldCode = fs.readFileSync(fullPath, {
      encoding: 'utf8'
    });
    const newCode = result.stdout;

    if (oldCode !== newCode)
    {
      printError(`File '${fullPath}' was not formatted`);
    }
  }
}

async function runDoxygen()
{
  console.log('Running Doxygen');
  if (!await verifyFileExists(paths.doxygen))
  {
    return;
  }

  const doxygenOptions = {
    cwd: dirs.root,
    stdio: ['ignore', 'pipe', 'pipe'],
    out: printLog,
    err: printError,
    reject: false
  };
  await exec(paths.doxygen, [], doxygenOptions);
}

async function runCmake()
{
  console.log('Running Cmake');
  if (!await verifyFileExists(paths.cmake))
  {
    return;
  }

  const cmakeOptions = {
    cwd: dirs.tempBuild,
    stdio: ['ignore', 'pipe', 'pipe'],
    out: printLog,
    err: printError,
    reject: false
  };
  await exec(paths.cmake, [dirs.root], cmakeOptions);
}

async function runBuild(config)
{
  console.log('Running Build');
  const testPaths = [];
  if (!await verifyFileExists(paths.cmake))
  {
    return testPaths;
  }

  const options = {
    cwd: dirs.tempBuild,
    stdio: ['ignore', 'pipe', 'pipe'],
    out: text =>
    {
      if (text.search(/FAILED/) === -1)
      {
        printLog(text);
      }
      else
      {
        printError(text);
      }
    },
    err: printError,
    reject: false
  };

  await exec(paths.cmake, ['--build', '.', '--config', config], options);

  if (platform === 'windows')
  {
    const executable = path.join(dirs.tempBuild, config, 'ne.exe');
    testPaths.push(executable);
  }
  else
  {
    const executable = path.join(dirs.tempBuild, 'ne');
    try
    {
      fs.chmodSync(executable, 0o777);
    }
    catch (err)
    {
      printError(err);
    }
    //testPaths.push(executable);
  }
  return testPaths;
}

async function runTests(testPaths)
{
  console.log('Running Tests');
  // TODO(Trevor.Sundberg): ne_io_input read test fails if we don't specify 'inherit' for stdin.
  for (const testPath of testPaths)
  {
    const inputPath = path.join(dirs.packages, 'test_io', 'input.txt');
    const outputPath = path.join(dirs.tempTest, 'output.txt');

    const input = fs.openSync(inputPath, 'r');
    const output = fs.openSync(outputPath, 'w');

    const options = {
      cwd: path.dirname(testPath),
      stdio: [input, output, 'pipe'],
      err: printError,
      reject: false
    };
    await exec(testPath, ['--simulated'], options);

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
      printError(`Standard output does not match: '${outputText}'`);
    }
  }
}

async function createImage()
{
  console.log('Creating');
  makeAllDirs();
  const imagePath = path.join(dirs.tempImages, `image-${platform}.zip`);
  safeDeleteFile(imagePath);

  const output = fs.createWriteStream(imagePath);
  const archive = archiver('zip', {
    zlib:
    {
      level: 9
    }
  });
  archive.on('error', printError);
  archive.on('warning', printError);

  let lastTime = performance.now();
  archive.on('progress', data =>
  {
    const now = performance.now();
    if (now - lastTime > 1000)
    {
      const percent = getPercent(data.fs.processedBytes, data.fs.totalBytes);
      console.log(` - ${percent}% - ${data.fs.processedBytes} / ${data.fs.totalBytes}`);
      lastTime = now;
    }
  });

  archive.pipe(output);
  archive.directory(dirs.tempTools, false);
  archive.finalize();

  await new Promise(resolve => output.on('close', resolve));
  console.log('Created');
}

async function clean()
{
  console.log('Cleaning');
  rimraf.sync(dirs.temp);
  console.log('Cleaned');
}

async function build()
{
  console.log('Building');
  makeAllDirs();
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
  const testPaths = await runBuild('Release');
  await runTests(testPaths);
  console.log('Built');
}

async function installImage()
{
  makeAllDirs();
  await installProgram({
    name: 'Image',
    windows: {
      url: 'https://github.com/TrevorSundberg/ne_files/releases/download/v0.0.0/image-windows.zip',
      file: 'image-windows.zip',
      install: filePath => zipInstall(filePath, dirs.tempTools)
    },
    linux: {
      url: 'https://github.com/TrevorSundberg/ne_files/releases/download/v0.0.0/image-linux.zip',
      file: 'image-linux.zip',
      install: filePath => zipInstall(filePath, dirs.tempTools)
    },
  });
}

async function installSources()
{
  makeAllDirs();
  await installLlvm();
  await installDoxygen();
  await installCmake();
}

async function main()
{
  await setupEnvironment();

  const commands = {
    createImage,
    clean,
    installImage,
    installSources,
    build,
  };

  let args = process.argv.slice(2);
  if (args.length === 0)
  {
    args = ['clean', 'installImage', 'build'];
  }

  for (const arg of args)
  {
    console.log('--------------------------------------------------');
    const command = commands[arg];
    if (command)
    {
      await command();
    }
    else
    {
      printError(`Invalid command '${arg}'`);
    }
  }
}
main();
