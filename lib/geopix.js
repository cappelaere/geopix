
try {
	module.exports = require('../build/Release/geopix');
} catch (e) {
	module.exports = require('../build/default/geopix');
}