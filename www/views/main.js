function GenCardHTML(parent , id) {  
  var div = document.createElement("div");   
  div.innerHTML = 
  `<div class="mdc-card card-book" id="card-${id}" data-img = "img/1.jpg">
  <div class="mdc-card__primary-action" tabindex="0" id="card-${id}-content" style="flex-direction:row;">
    <div class="mdc-card__media card-image-container-normal" id="card-${id}-image-container">
      <img src="https://i.ibb.co/2qTJ0CS/1.jpg" id="card-${id}-img" style="width:100%;" crossOrigin />
    </div>
    
    <div style="flex-direction:column;">
      <div style="padding: 1rem;">
        <h2 class="mdc-typography--headline6 card-title" id="card-${id}-title">Out of the Box</h2>
        <h3 class="mdc-typography--subtitle2 card-subtitle" id="card-${id}-subtitle">by Suzanne Dudley</h3>
      </div>
      <div class="mdc-typography--body2 card-support" id="card-${id}-support">Visit ten places on our planet that are undergoing the biggest changes today.</div>
    </div>
    
  </div>
  <div class="mdc-card__actions" id="card-${id}-actions">
    <div class="mdc-card__action-buttons">
      <button class="mdc-button mdc-card__action mdc-card__action--button" id="card-${id}-button-1" data-card="card-${id}">Detail</button>
      <button class="mdc-button mdc-card__action mdc-card__action--button" id="card-${id}-button-2" disabled>Borrow</button>
    </div>
    
    <div class="mdc-card__action-icons">
      <button class="mdc-icon-button mdc-card__action mdc-card__action--icon" aria-pressed="false" aria-label="Add to favorites" title="Add to favorites">
        <i class="material-icons mdc-icon-button__icon mdc-icon-button__icon--on">favorite</i>
        <i class="material-icons mdc-icon-button__icon">favorite_border</i>
      </button>
      <button class="mdc-icon-button material-icons mdc-card__action mdc-card__action--icon" title="Share">share</button>
      <button class="mdc-icon-button material-icons mdc-card__action mdc-card__action--icon" title="More options" data-mdc-ripple-is-unbounded="true">more_vert</button>
    </div>
    
  </div>
</div>
`;
parent.appendChild(div);
mdc.ripple.MDCRipple.attachTo(document.querySelector(`#card-${id}-content`));
mdc.ripple.MDCRipple.attachTo(document.querySelector(`#card-${id}-button-1`));
mdc.ripple.MDCRipple.attachTo(document.querySelector(`#card-${id}-button-2`));
document.getElementById(`card-${id}-button-1`).addEventListener("click", cardHelper.expand_card.bind(cardHelper))
}