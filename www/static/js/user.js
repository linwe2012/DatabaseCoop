'use strict'

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

var Borrowed = []
var StaleBorrow = []
async function FetchAllBorrowed()
{
    var borr = await $post({
        url: global.baseurl + '/api/getborrowed',
        params: ''
    })
    borr = JSON.parse(borr)
    Borrowed = borr.payload.borrow
    if(!borr.ok) return;
    var i = 0
    for(let bo of Borrowed) {
        let res = await $post({
            url: global.baseurl + '/api/getbookabstract_by_isbn',
            params: `isbn_start=${bo.ISBN}&cnt=1&request_copy_info=false`
        })
        Borrowed[i]['bookinfo'] = JSON.parse(res)
        ++i
    }

    StaleBorrow = borr.payload.stale_borrow
    i = 0
    for(let bo of StaleBorrow) {
        let res = await $post({
            url: global.baseurl + '/api/getbookabstract_by_isbn',
            params: `isbn_start=${bo.ISBN}&cnt=1&request_copy_info=false`
        })
        StaleBorrow[i]['bookinfo'] = JSON.parse(res)
        ++i
    }
}

function RenderOneCard(bo, offset, if_show_action = true, due_text = 'Due at') {
  let b = bo.bookinfo[0]
  var cc = document.getElementById('card-container')

  var author = ''
  let i =0
  for(let au of b.authors) {
    if(i != 0) {
      author += ', '
    }
    author +=  au.author_name
    ++i
  }
  var d  = document.createElement('div')

  
  b.description = b.description || ''
  b.description = b.description.replace(/<\/?[^>]+(>|$)/g, "");
  var trimmed_desc = trimmSentence(b.description, 80, '...', ' ')
  author = trimmSentence(author, 34, ' et al.', ' ')
  let btime = TimeStampToString(bo.borrow_time)
  let due = TimeStampToString(bo.return_time)

  var action = ''
  if(if_show_action) {
    action = `
    <div class='card-inner-extra-action'>
      <button data-isbn='${b.ISBN}' data-copyid='${bo.copy_id}' onclick='RenewBook(this)'>Renew</button>
      <button data-isbn='${b.ISBN}' data-copyid='${bo.copy_id}' onclick='ReturnBook(this)'>Return</button>
    </div>`
  }
  
  var clip_class = ''
  var clip_txt = ''
  if(bo.status == 'overdue') {
    clip_class = 'clip__error'
    clip_txt = 'Overdue'
  }
  else if(bo.status == 'warning') {
    clip_class = 'clip__warning'
    clip_txt = 'Renewable'
  }
  d.innerHTML =  `
  <div class='card' id='${b.ISBN}'>
    <div class='card-img-wrap'>
      <img src=${b.img_url} />
    </div>
    <button class='card-favicon'>
      <i class="material-icons">bookmark_border</i>
    </button>
    <div class='card-inner'>
      <div class='clip ${clip_class}'>${clip_txt}</div>
      <h3 onclick='ShowPopup(this)' data-offset=${offset} style='cursor: pointer;'>
        ${b.title}
      </h3>
      <div class='card-inner-text'>
        ${trimmed_desc}
      </div>
      <div class='card-inner-extra'>
        By ${author}
        <div class='card-inner-extra-auxtext'>Borrowed at ${btime}</div>
        <div class='card-inner-extra-auxtext' id='due-${b.ISBN}-${bo.copy_id}'>${due_text} <em>${due}</em></div>
        ${action}
      </div>
    </div>
  </div>
  `

  cc.appendChild(d)
}


async function RenderPage() {
    await FetchAllBorrowed()
    console.log(Borrowed)
    let i = 0
    for(let b of Borrowed) {
        RenderOneCard(b, i)
        ++i
    }
    
    let h1 = document.createElement('h1')
    h1.style = 'text-align: center; width:100%;'
    //h1.style.width = '80%';
    // h1.style.margin = '20px auto';
    h1.innerHTML = 'History'
    $id('card-container').appendChild(h1)

    for(let b of StaleBorrow) {
      RenderOneCard(b, i, false, 'Returned at')
    }
    $id('user-info-extra').innerHTML = `<i class='material-icons'>school</i><span>${Borrowed.length} books borrowed</span>`
}


RenderPage()


async function ReturnRenewHelper(self, api) {
  var res = await $post ({
    url: global.baseurl + api,
    params: `isbn=${self.dataset.isbn}&copy_id=${self.dataset.copyid}`
  })

  res = JSON.parse(res)

  if(!res.ok) {
    ReportError(self, res, 5000)
    return res
  }
  ReportSuccess(self, 3000)
  return res
}

async function RenewBook(self) {
  let res = await ReturnRenewHelper(self, '/api/renewbook')
  if(res.ok) {
    $id(`due-${self.dataset.isbn}-${self.dataset.copyid}`).innerHTML = 'Due at <em>' + TimeStampToString(res.payload.new_date) + '</em>'
  }
}


async function ReturnBook(self) {
  await ReturnRenewHelper(self, '/api/returnbook')
}