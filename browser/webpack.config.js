const path = require('path');

const PATHS = {
    build: path.resolve(__dirname, 'build'),
    components: path.resolve(__dirname, 'components'),
    styles: path.resolve(__dirname, 'styled')
};

module.exports = {
    entry: PATHS.components + '/App.jsx',

    output: {
        path: PATHS.build,
        filename: 'genesis.pack.js'
    },

    resolve: {
        root: path.resolve(__dirname),
        extensions: ['', '.js', '.jsx', '.scss']
    },

    module: {
        loaders: [
            {
                test: /\.jsx?$/, // Process JS and JSX
                loader: 'babel',
                query:
                {
                    presets:['es2015', 'react']
                }
            },
            {
                test: /\.scss$/,
                loader: 'style!css!sass',//loaders: ['style', 'css', 'sass']
            }
        ]
    }
}