document.addEventListener('DOMContentLoaded', () => {
    const searchInput = document.getElementById('searchInput');
    const header = document.getElementById('header');
    const loader = document.getElementById('loader');
    const resultsArea = document.getElementById('resultsArea');
    const resultsList = document.getElementById('resultsList');
    const searchStats = document.getElementById('searchStats');

    // Handle "Enter" key press
    searchInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            const query = searchInput.value.trim();
            if (query) {
                performSearch(query);
            }
        }
    });

    async function performSearch(query) {
        // Transition UI to search mode
        header.classList.add('searched');
        resultsArea.classList.remove('visible');
        loader.style.display = 'flex';
        
        const startTime = performance.now();

        try {
            // Fetch results from our Node.js Microservice API
            const response = await fetch(`/api/search?q=${encodeURIComponent(query)}`);
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            
            const results = await response.json();
            const endTime = performance.now();
            const timeTakenMs = (endTime - startTime).toFixed(1);

            renderResults(results, timeTakenMs);
        } catch (error) {
            console.error('Error fetching search results:', error);
            loader.style.display = 'none';
            searchStats.textContent = 'An error occurred while searching. Is the C++ Daemon running?';
            resultsList.innerHTML = '';
            resultsArea.classList.add('visible');
        }
    }

    function renderResults(results, timeTakenMs) {
        loader.style.display = 'none';
        resultsList.innerHTML = ''; // Clear previous results

        searchStats.textContent = `Found ${results.length} results in ${timeTakenMs} ms`;

        if (results.length === 0) {
            resultsList.innerHTML = `
                <div class="result-card" style="text-align: center; padding: 3rem;">
                    <h3 style="color: var(--text-secondary);">No results found.</h3>
                </div>
            `;
        } else {
            results.forEach(result => {
                // Wikipedia titles can be extracted nicely from the URL
                const rawTitle = result.url.split('/').pop() || result.url;
                const cleanTitle = decodeURIComponent(rawTitle).replace(/_/g, ' ');

                const card = document.createElement('a');
                card.href = result.url;
                card.target = "_blank";
                card.className = 'result-card';
                card.innerHTML = `
                    <div class="result-url">
                        <span>${result.url}</span>
                        <span class="result-score">TF-IDF: ${result.score.toFixed(2)}</span>
                    </div>
                    <div class="result-title">${cleanTitle}</div>
                `;
                resultsList.appendChild(card);
            });
        }

        // Trigger animation
        setTimeout(() => {
            resultsArea.classList.add('visible');
        }, 50);
    }
});
