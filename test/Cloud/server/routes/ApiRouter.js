'use strict';

const router = require('express').Router();

router.use(require('./DevicesRouter'));
router.use(require('./HistoryRouter'));
router.use(require('./BatchRouter'));
router.use(require('./FileManagerRouter'));

module.exports = router;