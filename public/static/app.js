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
        const checkbox = document.getElementById(`done-${id}`);
        const newDoneValue = checkbox.checked;
        const li = checkbox.closest("li");
        const text = li.querySelector("a").textContent;
        updateTask(id, text, newDoneValue);
        if (newCheckBox.checked) {
          textElement.style.textDecoration = "line-through";
        } else {
          textElement.style.textDecoration = "underline";
        }
      });

      editBtn.textContent = "edit";
      editBtn.className = "Btn";
      editBtn.id = `editBtn-${element.id}`;
      btnDiv.appendChild(editBtn);
      editBtn.addEventListener("click", function () {
        const parts = editBtn.id.split("-");
        const id = parseInt(parts[1]);
        editTask(id);
      });

      delBtn.textContent = "x";
      delBtn.className = "Btn";
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

  setTimeout(() => {
    todoList.innerHTML = "";
    fetchData();
  }, 1000);
}

function updateTask(id, text, newDoneValue) {
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

  setTimeout(() => {
    todoList.innerHTML = "";
    fetchData();
  }, 1000);
}

add.onclick = addTask;

todoInput.addEventListener("keydown", function (event) {
  if (event.key === "Enter") {
    addTask();
  }
});

function editTask(id) {
  const btnDiv = document.getElementById(`task-${id}`);
  const li = btnDiv.closest("li");
  const textElement = li.querySelector("a");

  const input = document.createElement("input");
  input.type = "text";
  input.value = textElement.textContent;
  input.className = "editInput";
  input.style.marginRight = "8px";

  const saveBtn = document.createElement("button");
  saveBtn.textContent = "Save";
  saveBtn.className = "Btn";

  li.insertBefore(input, textElement);
  li.insertBefore(saveBtn, btnDiv);
  textElement.style.display = "none";

  input.focus();

  saveBtn.onclick = function () {
    const newText = input.value.trim();
    const doneValue = li.querySelector(".checkBox").checked;

    li.removeChild(input);
    li.removeChild(saveBtn);
    textElement.style.display = "";

    if (newText !== "") {
      textElement.textContent = newText;
      updateTask(id, newText, doneValue);
      setTimeout(() => {
        todoList.innerHTML = "";
        fetchData();
      }, 1000);
    }
  };

  input.addEventListener("keydown", function (event) {
    if (event.key === "Enter") {
      saveBtn.click();
    }
  });
}

fetchData();
