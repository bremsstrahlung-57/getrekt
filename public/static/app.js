const todoInput = document.getElementById("todoInput");
const addBtn = document.getElementById("addBtn");
const todoList = document.getElementById("todoList");

async function fetchData() {
  try {
    const response = await fetch("http://localhost:8080/api/todos");
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();

    for (let index = 0; index < data.length; index++) {
      const element = data[index];

      const newLi = document.createElement("li");
      const newA = document.createElement("a");
      const editBtn = document.createElement("btn");
      const delBtn = document.createElement("button");
      const newCheckBox = document.createElement("input");
      const btnDiv = document.createElement("div");

      newA.textContent = element.task;
      newLi.appendChild(newA);

      newCheckBox.setAttribute("type", "checkbox");
      newCheckBox.className = "checkBox";
      newCheckBox.id = `done-${element.id}`;
      newCheckBox.checked = element.done;
      if (element.done) {
        const textElement = newLi.querySelector("a");
        textElement.style.textDecoration = "line-through";
      }
      btnDiv.appendChild(newCheckBox);
      newCheckBox.addEventListener("change", function () {
        const textElement = newLi.querySelector("a");
        const parts = newCheckBox.id.split("-");
        const id = parseInt(parts[1]);
        checkTask(id);
        if (newCheckBox.checked) {
          textElement.style.textDecoration = "line-through";
        } else {
          textElement.style.textDecoration = "underline";
        }
      });

      editBtn.textContent = "edit";
      editBtn.className = "deleteBtn";
      editBtn.id = "editBtn";
      btnDiv.appendChild(editBtn);

      delBtn.textContent = "x";
      delBtn.className = "deleteBtn";
      delBtn.id = `deleteBtn-${element.id}`;
      btnDiv.appendChild(delBtn);
      delBtn.addEventListener("click", function () {
        const parts = delBtn.id.split("-");
        const id = parseInt(parts[1]);
        deleteTask(id);
      });

      btnDiv.id = `task-${element.id}`;
      newLi.appendChild(btnDiv);
      todoList.appendChild(newLi);
    }
  } catch (error) {
    console.error("Error fetching data: ", error);
  }
}

function deleteTask(id) {
  fetch(`http://localhost:8080/api/todos/${id}`, {
    method: "DELETE",
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      return response.text();
    })
    .then((data) => {
      console.log("Delete successful:", data);
    })
    .catch((error) => {
      console.error("Error during DELETE request: ", error);
    });
}

function checkTask(id) {
  const checkbox = document.getElementById(`done-${id}`);
  const newDoneValue = checkbox.checked;

  const li = checkbox.closest("li");
  const text = li.querySelector("a").textContent;

  fetch(`http://localhost:8080/api/todos/${id}`, {
    method: "PUT",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ text: text, done: newDoneValue }),
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      return response.text();
    })
    .then((data) => {
      console.log("Update successful:", data);
    })
    .catch((error) => {
      console.error("Error during PUT request: ", error);
    });
}

const add = document.getElementById("addBtn");

function addTask() {
  let inputField = document.getElementById("todoInput");
  var text = inputField.value.trim();
  if (text === "") {
    alert("Task cannot be empty");
    return;
  }
  console.log(text);

  fetch("http://localhost:8080/api/todos", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({ text: text }),
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      return response.text();
    })
    .then((data) => {
      console.log("Added successful:", data);
    })
    .catch((error) => {
      console.error("Error during POST request: ", error);
    });

  inputField.value = "";
}

add.onclick = addTask;

todoInput.addEventListener("keydown", function (event) {
  if (event.key === "Enter") {
    addTask();
  }
});

fetchData();
