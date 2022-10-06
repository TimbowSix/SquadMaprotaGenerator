## prod stage
FROM node:17-alpine as prod
RUN mkdir -p /home/node/app/node_modules && chown -R node:node /home/node/app
WORKDIR /home/node/app
USER node

ARG NODE_ENV=production
ENV NODE_ENV $NODE_ENV

COPY --chown=node:node package*.json ./
RUN npm clean-install && npm cache clean --force
COPY --chown=node:node . .

CMD [ "node", "main.js" ]
