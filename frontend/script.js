document.addEventListener('DOMContentLoaded', () => {
    
    const menuListDiv = document.getElementById('menu-list');
    const orderForm = document.getElementById('order-form');
    const orderStatusDiv = document.getElementById('order-status');
    const waitingTimeSpan = document.getElementById('waiting-time');
    const feedbackSection = document.getElementById('feedback-section'); 

    let allMenuItems = []; 
    let currentCustomerName = ""; 

    // --- 1. Fetch and Display Menu ---
    async function fetchMenu() {
        try {
            // Note: The script does not need to change the port here, as it uses relative path /menu
            const response = await fetch('/menu'); 
            if (!response.ok) throw new Error(`HTTP error! Status: ${response.status}`);
            
            allMenuItems = await response.json(); 

            menuListDiv.innerHTML = ''; 

            if (allMenuItems.length === 0) {
                menuListDiv.innerHTML = '<p>No menu items available.</p>';
                return;
            }

            allMenuItems.forEach(item => {
                const itemDiv = document.createElement('div');
                itemDiv.innerHTML = `
                    <span class="menu-item-name">${item.id}. ${item.name}</span>
                    <span class="prep-time">(${item.prepTime} mins)</span>
                `;
                menuListDiv.appendChild(itemDiv);
            });

        } catch (error) {
            console.error('Error fetching menu:', error);
            menuListDiv.innerHTML = `<p style="color: red;">Error loading menu from C++ server: ${error.message}</p>`;
        }
    }

    // --- 2. Handle Order Submission ---
    orderForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        currentCustomerName = document.getElementById('customer-name').value;
        const dishInput = document.getElementById('dish-selection').value;
        
        const dish_ids = dishInput.split(',')
            .map(s => parseInt(s.trim()))
            .filter(id => !isNaN(id) && id > 0 && id <= allMenuItems.length);

        if (dish_ids.length === 0) {
            orderStatusDiv.innerHTML = '<span style="color: red;">Please select at least one valid dish number.</span>';
            return;
        }

        orderStatusDiv.innerHTML = 'Placing order...';
        
        try {
            const response = await fetch('/order', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ customer: currentCustomerName, dish_ids })
            });

            const result = await response.json();

            if (!response.ok) {
                orderStatusDiv.innerHTML = `<span style="color: red;">Error: ${result.error || 'Could not place order.'}</span>`;
                return;
            }

            // Success: Display confirmation and start timers
            orderStatusDiv.innerHTML = `
                <div style="background-color: #d4edda; color: #155724; padding: 10px; border-radius: 5px;">
                    ✅ **Order #${result.id} Confirmed!** <br>
                    Total Prep Time: **${result.prepTime} mins**. <br>
                    Estimated Ready Time: **${result.totalWaitTime} minutes**.
                </div>
                <div id="countdown-status" style="margin-top: 15px;"></div>
                <div id="cancel-status" style="margin-top: 10px;"></div>
            `;
            
            startOrderTimer(result.prepTime, result.cancelWindow, result.id, result.customer);
            
            waitingTimeSpan.textContent = result.totalWaitTime;

        } catch (error) {
            console.error('Order submission error:', error);
            orderStatusDiv.innerHTML = '<span style="color: red;">Network error. Could not connect to C++ server.</span>';
        }
    });

    // --- 3. Timer and Feedback Simulation ---
    function startOrderTimer(prepMinutes, cancelWindowMinutes, orderId, customerName) {
        // Use seconds for a quick demo: 1 min = 3 seconds
        const DEMO_FACTOR = 3; 
        let totalTimeSeconds = prepMinutes * DEMO_FACTOR;
        let cancelTimeSeconds = cancelWindowMinutes * DEMO_FACTOR;
        let elapsedSeconds = 0;

        const countdownDiv = document.getElementById('countdown-status');
        const cancelDiv = document.getElementById('cancel-status');

        const timerInterval = setInterval(() => {
            elapsedSeconds++;
            
            // Order Completion Logic
            let remainingTime = totalTimeSeconds - elapsedSeconds;
            if (remainingTime > 0) {
                let remainingMinutes = Math.ceil(remainingTime / DEMO_FACTOR);
                countdownDiv.innerHTML = `
                    🍽️ Order Preparation: **${remainingMinutes} minutes** remaining.
                `;
            } else {
                clearInterval(timerInterval);
                countdownDiv.innerHTML = `
                    🎉 **Order #${orderId} is READY for collection!**
                `;
                cancelDiv.innerHTML = '';

                promptFeedback(customerName);
            }

            // Cancellation Window Logic
            let remainingCancel = cancelTimeSeconds - elapsedSeconds;
            if (remainingCancel > 0) {
                let remainingCancelMinutes = Math.ceil(remainingCancel / DEMO_FACTOR);
                cancelDiv.innerHTML = `
                    **🔔 Cancellation window:** ${remainingCancelMinutes} minutes remaining.
                    <button id="cancel-btn" style="margin-left: 10px; background-color: #c0392b; color: white; border: none; padding: 8px; border-radius: 4px; cursor: pointer;">Cancel Order</button>
                `;
                document.getElementById('cancel-btn').onclick = () => {
                    alert(`Order #${orderId} was cancelled successfully! (Simulated)`);
                    clearInterval(timerInterval);
                    cancelDiv.innerHTML = '<span style="color: #c0392b;">❌ Order Cancelled by User.</span>';
                    countdownDiv.innerHTML = '';
                };
            } else if (elapsedSeconds <= totalTimeSeconds && remainingTime > 0) {
                cancelDiv.innerHTML = '🚫 Cancellation window has closed.';
            }

        }, 1000); // Update every 1 second
    }

    // --- 4. Feedback Prompt ---
    function promptFeedback(customerName) {
        feedbackSection.innerHTML = `
            <h2>⭐ Share Your Feedback</h2>
            <form id="feedback-form">
                <p>We hope you enjoyed your meal, **${customerName}**! Please share your thoughts.</p>
                <textarea id="feedback-text" rows="3" required style="width: 100%; padding: 10px; box-sizing: border-box;"></textarea>
                <button type="submit">Submit Feedback</button>
            </form>
            <div id="feedback-status" style="margin-top: 10px;"></div>
        `;
        
        document.getElementById('feedback-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            const comment = document.getElementById('feedback-text').value;
            const statusDiv = document.getElementById('feedback-status');
            
            statusDiv.textContent = 'Submitting...';

            try {
                const response = await fetch('/feedback', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ customer: customerName, comment: comment })
                });

                if (response.ok) {
                    statusDiv.innerHTML = '<span style="color: #27ae60;">🙏 Thank you for your feedback! (Submitted to C++ Map)</span>';
                    document.getElementById('feedback-form').querySelector('button').disabled = true;
                    document.getElementById('feedback-text').disabled = true;
                } else {
                    statusDiv.innerHTML = '<span style="color: red;">Error submitting feedback.</span>';
                }

            } catch (error) {
                console.error('Feedback submission error:', error);
                statusDiv.innerHTML = '<span style="color: red;">Network error submitting feedback.</span>';
            }
        });
    }

    // Initialize the application: Load the menu
    fetchMenu();
});