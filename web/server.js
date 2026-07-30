const express = require('express');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');

const app = express();
app.use(cors());

// The path to the C++ executable
const buildDir = path.join(__dirname, '..', 'build');
const indexerPath = path.join(buildDir, 'indexer.exe');

console.log('Starting C++ Indexer Daemon...');
const indexer = spawn(indexerPath, ['--daemon'], { cwd: buildDir });

// A strict FIFO queue for pending web requests
const pendingRequests = [];

let buffer = '';

indexer.stdout.on('data', (data) => {
    buffer += data.toString();
    
    // Process all complete lines in the buffer
    let newlineIndex;
    while ((newlineIndex = buffer.indexOf('\n')) !== -1) {
        const line = buffer.slice(0, newlineIndex).trim();
        buffer = buffer.slice(newlineIndex + 1);
        
        if (!line) continue;
        
        // If it looks like our JSON output from the C++ daemon
        if (line.startsWith('[')) {
            // It's a JSON response! Resolve the oldest pending request
            if (pendingRequests.length > 0) {
                const req = pendingRequests.shift();
                try {
                    const json = JSON.parse(line);
                    req.res.json(json);
                } catch (e) {
                    req.res.status(500).json({ error: "Failed to parse indexer JSON" });
                }
            }
        } else {
            // It's a C++ startup log or warning, print to Node console
            console.log(`[C++] ${line}`);
        }
    }
});

indexer.stderr.on('data', (data) => {
    console.error(`[C++ ERR] ${data}`);
});

indexer.on('close', (code) => {
    console.log(`C++ Indexer crashed or exited with code ${code}`);
    process.exit(1);
});

// REST API Endpoint
app.get('/api/search', (req, res) => {
    const query = req.query.q;
    if (!query) {
        return res.status(400).json({ error: "Missing 'q' parameter" });
    }
    
    // Enqueue the request
    pendingRequests.push({ res, query });
    
    // Write query to C++ stdin. MUST end with \n
    indexer.stdin.write(query + '\n');
});

// Start Server
const PORT = 3000;
app.listen(PORT, () => {
    console.log(`\n================================`);
    console.log(`Node.js Search API running on:`);
    console.log(`http://localhost:${PORT}/api/search?q=computer+science`);
    console.log(`================================\n`);
});
