'use strict'

function trimmSentence(str, max, etc, delim) {
  var trimmed = str.substr(0, max)
  trimmed = trimmed.substr(0, Math.min(trimmed.length, trimmed.lastIndexOf(delim)))
  if(trimmed.length < str.length) {
    trimmed += etc
  }
  return trimmed
}
function BuildSmallCard(b, offset) {
  var cc = document.getElementById('card-container')
  var disabled = b.in_copies.length ? '' : 'disabled';
  var i = 0
  var author = ''
  for(let au of b.authors) {
    if(i != 0) {
      author += ', '
    }
    author +=  au.author_name
    ++i
  }
  var d  = document.createElement('div')
  d.style = 'break-inside: avoid;'
  b.description = b.description || ''
  b.description = b.description.replace(/<\/?[^>]+(>|$)/g, "");
  var trimmed_desc = trimmSentence(b.description, 80, '...', ' ')
  author = trimmSentence(author, 34, ' et al.', ' ')

  d.innerHTML = `
  <div class='card' id='${b.ISBN}'>
    <div class='card-img-wrap'>
      <img src=${b.img_url} />
    </div>
    <button class='card-favicon'>
      <i class="material-icons">bookmark_border</i>
    </button>
    <div class='card-inner'>
      <h3 onclick='ShowPopup(this)' data-offset=${offset} style='cursor: pointer;'>
        ${b.title}
      </h3>
      <div class='card-inner-text'>
        ${trimmed_desc}
      </div>
      <div class='card-inner-extra'>
        By ${author}
        <div class='card-inner-extra-auxtext'>${b.in_copies.length}/${b.num_total_copies} available</div>
        <!-- div class='card-inner-extra-action'>
              <button data-isbn='${b.ISBN}' ${disabled}>Borrow</button>
        </div-->
      </div>
    </div>
  </div>
  `
  cc.appendChild(d)
}

var Books = [];
var Branches = [];
async function RequestBooks(isbn_start, cnt) {
  var res = await $post({
    url: global.baseurl + '/api/getbookabstract_by_isbn',
    params: `isbn_start=${isbn_start}&cnt=${cnt}`
  })
  var rest = JSON.parse(res)

  for(let b of rest) {
    b['num_total_copies'] = b.in_copies.length + b.out_copies.length + b.not_copies.length
    BuildSmallCard(b, Books.length)
    Books.push(b)
  }
  // console.log(Books[0])
}

async function FetchAllBranches() {
  var res = await $post({
    url: global.baseurl + '/api/getallbranches',
    params: ''
  })

  res = JSON.parse(res)
  if(!res.ok) {
    console.error(res)
    return
  }

  Branches = res.payload
}

function ShowPopup(self) {
  BuildPopup(Books[self.dataset.offset])
}

function BuildPopup(b) {
  var authors = ''
  for(let au of b.authors) {
    au.author_img_url = au.author_img_url || '/static/img/avatar.png'
    authors += `
    <div class='author'>
      <div class='author-avatar-wrap'> <img src='${au.author_img_url}'></div>
      <p>${au.author_name}</p>
    </div>
    `
    $id('popup-authors').innerHTML = authors
  }

  var addDetail = (icon, text)=>{
    return `
    <div class='detail-item'>
      <i class='material-icons'>${icon}</i>
      <p>${text}</p>
    </div>
    `
  }
  var pd = b.date_published
  var detail = addDetail('account_balance', b.publisher + `,   ${pd.year}/${pd.month}/${pd.date}`)
  detail += addDetail('translate', b.language_code || 'Unkown')  
  detail += addDetail('print', b.format || 'Unkown')  
  detail += addDetail('attach_money0', b.price || 'Unkown')
  detail += addDetail('description', b.num_pages ? b.num_pages + ' pages' : 'Unkown')
  var ext = 'error_outline'
  if(b.in_copies.length > 0) ext = '_' + b.in_copies.length;
  if(b.in_copies.length > 9) ext += '_plus'

  detail += addDetail('filter'+ext, `${b.in_copies.length}/${b.num_total_copies} in circulation`)

  var incirc = ''
  for(let avai of b.in_copies) {
    incirc += `
    <div class='detail-item'>
      <i class='material-icons'>confirmation_number</i> <div style='margin: auto 0;'>${Branches[avai.branch_id].branch_name}</div>
      <button data-isbn='${b.ISBN}' data-copyid='${avai.copy_id}' onclick='BorrowBook(this)'>Borrow</button>
    </div>
    `
  }

  var outcir = ''

  for(let out of b.out_copies) {
    outcir += `
    <div class='detail-item'>
      <i class='material-icons'>replay</i> <div style='margin: auto 0;'>Due at ${TimeStampToString(out.return_time)}</div>
      <button data-isbn='${b.ISBN}' data-copyid='${out.copy_id}' onclick='RecallBook(this)'>Recall</button>
    </div>
    `
  }

  $id('popup-wrap').style.display = ''
  $id('popup-img').setAttribute('src', b.img_url)
  $id('popup-img-pop').setAttribute('src', b.img_url)
  $id('popup-title').innerHTML = b.title
  $id('popup-text').innerHTML = b.description
  $id('popup-details').innerHTML = detail
  $id('popup-in-circulation').innerHTML = (incirc == '') ? 'No Availables' : ('<h3>In Circulation</h3>'+incirc)
  $id('popup-out-circulation').innerHTML = (outcir == '') ? 'No Copies borrowed' : ('<h3>Borrowed</h3>'+outcir)
}



function ClosePopup(e) {
  if(e.target != this) return
  $id('popup-wrap').style.display = 'none'
}

function PopImage() {
  $id('popup-img-pop').classList = 'zoom-in__600'
  $id('popup-img-pop-wrap').style.display = 'block'
}

function ClosePopImage(){
  
  $id('popup-img-pop').classList = 'zoom-out__250'
  setTimeout(()=>{
    $id('popup-img-pop-wrap').style.display = 'none'
  }, 200);
}

async function BorrowBook(self) {
  var res = await $post({
    url: global.baseurl + '/api/borrowbook',
    params: `isbn=${self.dataset.isbn}&copy_id=${self.dataset.copyid}`
  })
  self.innerHTML = 'Requesting...'
  console.log(res)
  res = JSON.parse(res)
  if(res.ok) { self.innerHTML = 'Succeed' }
  else { self.innerHTML = res.msg}
  setTimeout(()=>{
    self.innerHTML = 'Borrow'
  }, 5000)
}
var isDone = false

async function loadmore() {
  if(isDone) { $id('loadmore').style.display = 'none'; return}
  var last_len = Books.length;
  await RequestBooks(Books[Books.length-1].ISBN, 20)
  if(Books.length == last_len) { isDone = true; $id('loadmore').innerHTML = 'End Of Pages'}
}

async function RecallBook(self) {
  let res = await $post({
    url: global.baseurl + '/api/recallbook',
    params: `isbn=${self.dataset.isbn}`
  })

  res = JSON.parse(res)

  if(!res.ok) {
    ReportError(self, res, 5000)
    return
  }

  ReportSuccess(self, 3000)
}

async function Init()
{
  let fetbr = FetchAllBranches()
  var search = await $post({
    url: global.baseurl + '/api/fetchsearch',
    params: ''
  })

  search = JSON.parse(search)
  console.log(search)
  await fetbr;
  if(search.ok) {
    isDone = true
    for(let b of search.payload.book) {
      RequestBooks(b.ISBN, 1)
    }
    return
  }
  RequestBooks(0, 60)
}

Init()
$id('popup-wrap').addEventListener('click', ClosePopup)
$id('popup-close').addEventListener('click', ClosePopup)

