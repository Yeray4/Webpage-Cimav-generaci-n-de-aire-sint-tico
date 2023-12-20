function redirect() {//funcion para redireccionar a la pagina de estados de mexico
    var seleccion = document.getElementById("opcionesestados").value;
    if (seleccion !== '') {
        window.location.href = seleccion;
        console.log(seleccion);
    }
}

/* document.addEventListener("DOMContentLoaded", function() {
    var valueToSend = "0"; 
    var url = "/ubi?ubi=" + encodeURIComponent(valueToSend);
    
    var xhttp = new XMLHttpRequest();
    
    xhttp.onreadystatechange = function() {
      if (xhttp.readyState == 4 && xhttp.status == 200) {
        console.log("Valor enviado al servidor con éxito");
      }
    };
    
    xhttp.open("GET", url, true);
    xhttp.send();
  
  
    
  }); */

