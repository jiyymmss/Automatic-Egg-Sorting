
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1.0" />
<title>Login | Egg Sorting</title>
<link rel="stylesheet" href="style.css" />
</head>
<body class="centered">
<div class="card">
<center>
<h2>Egg Sorting & Pet Feeding</h2>
</center>
<form id="loginForm">
<input type="text" placeholder="Username" id="username" required />
<input type="password" placeholder="Password" id="password" required />
<button type="submit">Login</button>
</form>
<p id="error"></p>
</div>


<script>
const form = document.getElementById('loginForm');
const error = document.getElementById('error');

form.addEventListener('submit', async (e) => {
  e.preventDefault();

  const res = await fetch('api/login.php', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      username: document.getElementById('username').value,
      password: document.getElementById('password').value
    })
  });

  const data = await res.json();

  if (data.success) {
    window.location.href = 'dashboard.php';
  } else {
    error.innerText = data.message;
  }
});
</script>

</body>
</html>