function $id(id) {
    return document.getElementById(id);
}

function $post(obj) {
    return new Promise((resolve, reject) => {
        let xhttp = new XMLHttpRequest();
        // console.log(obj.url)
        xhttp.open('POST', obj.url, true);
        xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        
        xhttp.onreadystatechange = function() {
            if (xhttp.readyState === 4) {
                if (xhttp.status === 200) {
                    resolve(xhttp.responseText);
                } else {
                    reject(); // Probably including some rejection reason
                }
            }
        };

        xhttp.send(obj.params);
    });
}


function trimmSentence(str, max, etc, delim) {
    var trimmed = str.substr(0, max)
    trimmed = trimmed.substr(0, Math.min(trimmed.length, trimmed.lastIndexOf(delim)))
    if(trimmed.length < str.length) {
      trimmed += etc
    }
    return trimmed
}

function TimeStampToString(t){
    if(t === undefined || t === null) return 'Unkown Date'
    return `${t.year}-${t.month}-${t.date} ${t.hour}:${t.minute}:${t.second}`
}


function ReportError(self, res, timeout) {
    timeout = timeout || 3000
    let org = self.innerHTML
    self.disabled = true
    self.innerHTML = res.msg
    self.style.border = '2px solid #ef1101';
    
    setTimeout(() => {
        self.innerHTML = org
        self.disabled = false
        self.style.border = '';
    }, timeout);
}

function ReportSuccess(self, timeout) {
    timeout = timeout || 3000
    let org = self.innerHTML
    self.disabled = true
    self.innerHTML = 'Success'
    self.style.border = '2px solid #00aa55';
    self.style.color = '#00aa55'
    setTimeout(() => {
        self.innerHTML = org
        self.disabled = false
        self.style.border = '';
        self.style.color = ''
    }, timeout);
}


var global = {
    baseurl: 'http://127.0.0.1:3000'
}

