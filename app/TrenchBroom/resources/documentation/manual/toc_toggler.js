(() => {
  window.addEventListener("DOMContentLoaded", () => {
    const tocToggledClass = "toc_toggled";

    document.body.querySelector("div#toc_toggler .toc_toggler_button").addEventListener("click", (event) => {
      event.preventDefault();
      document.body.classList.toggle(tocToggledClass);
    });

    document.body.querySelectorAll("a").forEach((element) => {
      element.addEventListener("click", (event) => {
        document.body.classList.remove(tocToggledClass);
      });
    });
  });
})()
