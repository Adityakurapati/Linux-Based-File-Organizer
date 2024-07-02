const express = require('express');
const app = express();
const { exec } = require('child_process');
const path = require('path');
const multer = require('multer');
const bodyParser = require('body-parser');
const fs = require('fs').promises;

const upload = multer();
let directoryPath;

app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static('public'));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'views', 'index.html'));
});

app.post('/sort', upload.none(), async (req, res) => {
    const { sortCriteria, dirPath } = req.body;
    directoryPath = dirPath;

    console.log("Sort Criteria: ", sortCriteria);
    console.log("Directory Path: ", directoryPath);

    try {
        const stat = await fs.stat(directoryPath);
        if (!stat.isDirectory()) {
            throw new Error('Provided path is not a directory');
        }

        const command = `${__dirname}/Heap_sort "${directoryPath}" ${sortCriteria}`;
        exec(command, async (error, stdout, stderr) => {
            if (error) {
                console.error(`Error executing C program: ${stderr}`);
                res.status(500).send('Error executing C program');
                return;
            }

            console.log('C program executed successfully.');

            try {
                // const sortedFilenames = stdout.split('\n').map(line => line.trim()).filter(line => line !== '');
                // console.log("Sorted File Names:");
                // console.log(sortedFilenames);

                // await rearrangeFiles(sortedFilenames);


                // Read the sorted filenames from the output
                const sortedFilenames = stdout.match(/Name: (\S+)/g).map(match => match.split(': ')[1].replace(/,/g, ''));

                console.log("Sorted File Names:");
                console.log(sortedFilenames);

                // Rearrange files in the directory based on sorted filenames
                await rearrangeFiles(sortedFilenames);
                res.send('Files rearranged successfully.');
            } catch (err) {
                console.error('Error rearranging files:', err);
                res.status(500).send('Error rearranging files');
            }
        });
    } catch (error) {
        console.error('Error:', error.message);
        res.status(500).send(error.message);
    }
});

async function rearrangeFiles(sortedFilenames) {
    try {
        const files = await fs.readdir(directoryPath);
        const filesMap = new Map();
        files.forEach(file => {
            filesMap.set(file, path.join(directoryPath, file));
        });

        const tempPrefix = 'temp_';
        for (let i = 0; i < sortedFilenames.length; i++) {
            const filename = sortedFilenames[i];
            const filePath = filesMap.get(filename);
            if (filePath) {
                const newFilename = `${tempPrefix}${i}_${filename}`;
                const newPath = path.join(directoryPath, newFilename);
                await fs.rename(filePath, newPath);
            }
        }

        for (let i = 0; i < sortedFilenames.length; i++) {
            const filename = sortedFilenames[i];
            const tempFilename = `${tempPrefix}${i}_${filename}`;
            const tempPath = path.join(directoryPath, tempFilename);
            const newPath = path.join(directoryPath, filename);
            if (await fs.access(tempPath)) {
                await fs.rename(tempPath, newPath);
            }
        }

        console.log('Files rearranged successfully.');
    } catch (error) {
        console.error('Error rearranging files:', error);
        throw error;
    }
}

const PORT = 3000;
app.listen(PORT, () => {
    console.log(`Server listening on port ${PORT}`);
});
