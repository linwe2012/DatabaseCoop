
var baseurl = 'http://127.0.0.1:3000'
function createGroup(group_id) {
    var group = document.createElement('div');
    group.classList.add('setting-group');
    group.setAttribute('id', group_id);
    return group;
}

function createSettingItem(title, ipt_id) {
    var item = document.createElement('div');
    item.classList.add('setting-group');
    itemm.innerHTML = 
    `<h4>${title}</h4>
    <div class='ipt'>
        <input id='${ipt_id}'></input>
        <span class='ipt-bar'></span>
    </div>
    `
}

var all = [
    ['Meta', 'language', 'meta_lang', 'title', 'meta_title',  'first publish date', 'first_publish_date'],
    ['Author', 'Name', 'author_name', 'nationality', 'nationality', 'birth', 'birth', 'death', 'death', 'description', 'author_desc']
]

var changed = {}

var editables = document.getElementsByClassName('editable');
for (var editable of editables) {
    editable.onfocus = function(){
        this.data_orgin = this.innerHTML;
    }
    editable.onblur = function() {
        if(this.data_orgin == this.innerHTML) return;
        // this.innerHTML = this.data_orgin;
        $id(this.dataset.table).querySelector(`tr[data-row='${this.dataset.row}']`).style.background = 'rgba(80,137,241,.1)' 
        if(this.dataset.table in changed) {
            changed[this.dataset.table].add(this.dataset.row)
        }
        else {
            changed[this.dataset.table] = new Set;
            changed[this.dataset.table].add(this.dataset.row)
        }
        console.log(changed);
    }
}

var submits = document.getElementsByClassName('submit');
for(var sub of submits) {
    sub.onclick = function() {
        var table = this.dataset.table
        if(!(table in changed)) {
            this.innerHTML = 'Nothing Changed'
            setInterval(() => {
                this.innerHTML = 'Submit'
            }, 2000);
            return
        }
        var table_el = $id(this.dataset.table)
        var res = []
        var i = 0
        for(let c of changed[table]) {
            var cols = table_el.querySelectorAll(`td[data-row='${c}']`)
            res.push(new Array)
            for(col of cols) {
                res[i].push(col.innerHTML)
            }
            ++i;
        }
        console.log(res)
    }
}

function loadBatchFromFile(self) {
    console.log(self.files.length)
    if (self.files.length == 0) return;
    var reader = new FileReader();
    reader.onload = async function(e) {
        // console.log(e.target.result)
        var res = await $post({
            url: baseurl + '/api/loadfromgoodread',
            params : e.target.result
        })
        
    }
    reader.readAsText(self.files[0])
}

async function generateRandomCopies() {
    var avg = document.getElementById('random-generate-avg').value
    var res = await $post({
        url: baseurl + '/api/randomcopiesgen',
        params: `avg=${avg}`
    })
}


/*
function loadBatchFromFile(self) {
    console.log(self.files)
    if (self.files.length){
        var file = self.files[0];
        console.log(file)
        var reader = new FileReader();
        reader.onload = function(e) {
                // The file's text will be printed here
                text = e.target.result
            var entries = JSON.parse(text)
            
            console.log(entries)
            var arr = []
            var i = 0
            for (let entry of entries) {
                arr.push(new Array)
                if(entry.title != undefined)
                    arr[i].push(entry.title)
                else arr[i].push(null)

                if(entry.subtitle != undefined)
                    arr[i].push(entry.subtitle)
                else arr[i].push(null)

                if(entry.languages != undefined && entry.languages[0].key != undefined)
                    arr[i].push(entry.languages[0].key)
                else arr[i].push(null)

                if(entry.isbn_13 != undefined)
                    arr[i].push(entry.isbn_13)
                else arr[i].push(null)
                
                // array
                if(entry.publishers != undefined)
                    arr[i].push(entry.publishers)
                else arr[i].push(null)

                if(entry.publish_country != undefined)
                    arr[i].push(entry.publish_country)
                else arr[i].push(null)

                if(entry.publish_date != undefined)
                    arr[i].push(entry.publish_date)
                else arr[i].push(null)

                if(entry.revision != undefined)
                    arr[i].push(entry.revision)
                else arr[i].push(null)
                
                if(entry.name != undefined)
                    arr[i].push(entry.name)
                else arr[i].push(null)

                if(entry.personal_name != undefined)
                    arr[i].push(entry.personal_name)
                else arr[i].push(null)

                if(entry.number_of_pages != undefined)
                    arr[i].push(entry.number_of_pages)
                else arr[i].push(null)

                // array
                if(entry.subjects != undefined)
                    arr[i].push(entry.number_of_pages)
                else arr[i].push(null)

                ++i
            }
            console.log(arr)
        }
        reader.readAsText(file)        
    }
}
*/



// console.log(tables)