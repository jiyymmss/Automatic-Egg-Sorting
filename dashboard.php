

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>Dashboard</title>
<link rel="stylesheet" href="style.css" />
<!-- DataTables CSS -->
<link rel="stylesheet" href="https://cdn.datatables.net/1.13.8/css/jquery.dataTables.min.css">

<!-- jQuery -->
<script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>

<!-- DataTables JS -->
<script src="https://cdn.datatables.net/1.13.8/js/jquery.dataTables.min.js"></script>

</head>
<body>
<nav class="navbar">
<h2>Egg Sorting & Pet Feeding</h2>
<ul>
<li onclick="showPage('schedule')">Pet Feeding</li>
<li onclick="showPage('eggs')">Egg Counts</li>
<li onclick="logout()">Logout</li>
</ul>
</nav>


<main>
<!-- PET FEEDING SCHEDULE -->
<section id="schedule" class="page">
<h3>Pet Feeding Schedule</h3>
<form id="scheduleForm">
<input type="time" id="feed_time" required />
<button>Add Schedule</button>
</form>
<ul id="scheduleList"></ul>
</section>


<!-- EGG COUNT TABLE -->
<section id="eggs" class="page hidden">
  <div class="header-flex">
  <h3>Egg Count Records</h3>

  <div class="totals-box">
    <div class="total-card small">
  <div class="label">Small</div>
  <div class="value" id="totalSmall">0</div>
</div>

<div class="total-card medium">
  <div class="label">Medium</div>
  <div class="value" id="totalMedium">0</div>
</div>

<div class="total-card large">
  <div class="label" style="text-align: center;">Large</div>
  <div class="value" id="totalLarge">0</div>
</div>

<div class="total-card total">
  <div class="label">Total Eggs</div>
  <div class="value" id="totalEggs">0</div>
</div>
  </div>
</div>



  <label for="filterDate">Filter by Date</label>
  <input type="date" id="filterDate" />

  <table id="eggDataTable" class="display">
    <thead>
      <tr>
        <th>Date</th>
        <th>Small</th>
        <th>Medium</th>
        <th>Large</th>
      </tr>
    </thead>
    <tbody></tbody>
  </table>
</section>


</main>


<script src="script.js"></script>
</body>
</html>