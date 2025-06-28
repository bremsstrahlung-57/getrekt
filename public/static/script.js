const todoInput = document.getElementById("todoInput");
const addBtn = document.getElementById("addBtn");
const todoList = document.getElementById("todoList");

function createTodoItem(text) {
  const li = document.createElement("li");
  li.textContent = text;
  const delBtn = document.createElement("button");
  delBtn.textContent = "✕";
  delBtn.className = "deleteBtn";
  delBtn.onclick = () => li.remove();
  li.appendChild(delBtn);
  li.onclick = (e) => {
    if (e.target !== delBtn) li.classList.toggle("done");
  };
  return li;
}

addBtn.onclick = () => {
  const text = todoInput.value.trim();
  if (text) {
    todoList.appendChild(createTodoItem(text));
    todoInput.value = "";
    todoInput.focus();
  }
};

todoInput.addEventListener("keydown", (e) => {
  if (e.key === "Enter") addBtn.onclick();
});
