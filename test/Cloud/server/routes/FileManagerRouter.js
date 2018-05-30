'use strict';

const router = require('express').Router();

const fileManagerController = require('../controllers/fileManagerController');
const validators = require('../modules/Validators');

router.route('/files/url')
    .post(validators.requestSignedURLValidator, fileManagerController.requestSignedURL);

module.exports = router;