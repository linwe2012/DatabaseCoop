'use strict'

function createCard(parent)
{
    var card = document.createElement('div');
    card.classList.add('card');
    card.innerHTML = `
        <div class='card-img-wrap'>
            <img src='https://i.ibb.co/2qTJ0CS/1.jpg' />
        </div>
        <button class='card-favicon'>
            <i class="material-icons">bookmark_border</i>
        </button>
        <div class='card-inner'>
            <h3>
             Great Amsterdam
            </h3>
            <div class='card-inner-text'>
                Okay, This is a text; It is expected to be very lng.... <br>
                Now we are trying to add more things.
            </div>
            <div class='card-inner-extra'>
                Written by Ysy
            <div class='card-inner-extra-auxtext'>This is auxiliary text. 28 counts.</div>
            <div class='card-inner-extra-action'>
                    <button>Borrow</button>
            </div>
        </div>
        </div>
    `
    parent.appendChild(card);
}
document.getElementById('topnav-spaceholder').style.display = 'none';
var lastScroll = window.pageYOffset;

window.onscroll = function() {
    var avatar_background_h = document.getElementById('avatar-background').clientHeight;
    if(avatar_background_h > window.pageYOffset + 100) {
        document.getElementById("topnav").style.background = '#ffffff44';
    }
    else {
        document.getElementById("topnav").style.background = '#fff';
    }
}

