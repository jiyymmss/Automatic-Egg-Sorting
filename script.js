// ------------------- NAVIGATION -------------------
function showPage(pageId) {
  const pages = document.querySelectorAll('.page');
  pages.forEach(page => page.classList.add('hidden'));

  document.getElementById(pageId).classList.remove('hidden');

  // Load totals when egg page is shown
  if (pageId === 'eggs') {
    loadEggTotals();
  }
}

function to12Hour(time24) {
  const [hour, minute] = time24.split(':');
  let h = parseInt(hour, 10);
  const ampm = h >= 12 ? 'PM' : 'AM';
  h = h % 12 || 12;
  return `${h}:${minute} ${ampm}`;
}

function logout() {
  alert('Logging out...');
  window.location.href = 'index.php';
}

// ------------------- PET FEEDING SCHEDULE -------------------
const scheduleForm = document.getElementById('scheduleForm');
const feedTimeInput = document.getElementById('feed_time');
const scheduleList = document.getElementById('scheduleList');

async function loadSchedules() {
  scheduleList.innerHTML = '';
  const res = await fetch('/eggsorting/api/schedule.php');
  const data = await res.json();

  data.forEach(item => {
    const li = document.createElement('li');

    const label = document.createElement('div');
    label.textContent = 'Feeding Time';

    const timeText = document.createElement('span');
    timeText.textContent = to12Hour(item.feed_time);
    timeText.style.fontWeight = '600';

    const deleteBtn = document.createElement('button');
    deleteBtn.textContent = '❌';

    deleteBtn.onclick = async () => {
      await fetch('/eggsorting/api/schedule.php', {
        method: 'DELETE',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ id: item.id })
      });
      loadSchedules();
    };

    li.append(label, timeText, deleteBtn);
    scheduleList.appendChild(li);
  });
}

scheduleForm.addEventListener('submit', async (e) => {
  e.preventDefault();
  await fetch('/eggsorting/api/schedule.php', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ feed_time: feedTimeInput.value })
  });
  feedTimeInput.value = '';
  loadSchedules();
});

loadSchedules();

// ------------------- EGG COUNT TABLE -------------------
let eggTableDT;

function initEggTable() {
  eggTableDT = $('#eggDataTable').DataTable({
    responsive: true,
    pageLength: 10,
    lengthChange: false,
    order: [[0, 'desc']],
    columns: [
      { data: 'date' },
      { data: 'small' },
      { data: 'medium' },
      { data: 'large' }
    ]
  });
}

async function loadEggs(date = '') {
  const res = await fetch(`/eggsorting/api/eggs.php?date=${date}`);
  const data = await res.json();
  eggTableDT.clear();
  eggTableDT.rows.add(data);
  eggTableDT.draw();
}

// ---------------- TOTAL EGG CARDS ----------------
function loadEggTotals(date = '') {
  $.ajax({
    url: '/eggsorting/api/egg_totals.php',
    method: 'GET',
    data: { date: date },
    dataType: 'json',
    success: function (totals) {
      // Update the card values
      $('#totalSmall').text(totals.small);
      $('#totalMedium').text(totals.medium);
      $('#totalLarge').text(totals.large);
      $('#totalEggs').text(totals.total);
    },
    error: function (xhr, status, error) {
      console.error('Error loading egg totals:', error);
      // Reset cards to 0 if error occurs
      $('#totalSmall').text(0);
      $('#totalMedium').text(0);
      $('#totalLarge').text(0);
      $('#totalEggs').text(0);
    }
  });
}
	

// ---------------- DATE FILTER ----------------
$('#filterDate').on('change', function () {
  const date = this.value;
  loadEggs(date);
  loadEggTotals(date);
});

// ---------------- INIT ----------------
$(document).ready(function () {
  initEggTable();
  loadEggs();

  setInterval(() => {
  const filterDate = $('#filterDate').val() || '';
  loadEggs(filterDate);
  loadEggTotals(filterDate);
}, 5000);
});
