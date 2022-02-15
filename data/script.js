fetch('temp')
  .then(body => body.text())
  .then(temp => {
    document.querySelector('#loader').style.display="None"
    document.querySelector('#temp').innerHTML = temp + ' °C'
    document.querySelector('#temp').style.display="block"
})
