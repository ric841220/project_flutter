const express = require('express');
const cors = require('cors');
const app = express();

const fileupload = require('express-fileupload');
app.use(fileupload(), cors())

const jsftp = require('jsftp');
const Ftp = new jsftp({
  host: '這邊填FTP的主機',
  port: 21, // FTP 的連接埠，預設是 21
  user: '這邊填使用者名稱',
  pass: '這邊填密碼'
});

app.post('/uploadFileToFTP', (req, res) => {
  const file = req.files.clientFile; // 傳來的檔案
  const fileName = file.name; // 檔名
  const fileData = file.data; // 要傳到 FTP 上的檔案
  Ftp.put(fileData, '這邊填寫要傳到FTP的哪個路徑上' + fileName, err => {
    if(!err) {
      res.send('Upload Done');
    } else {
      res.send(err);
    }
  });
});

const PORT = process.env.PORT || 8080;
app.listen(PORT);