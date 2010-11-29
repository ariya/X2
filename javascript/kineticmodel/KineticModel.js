/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

var KineticModel = (function () {

    KineticModel = function (min, max, handler)
    {
        this.ticker = null;

        this.minimum = min;
        this.maximum = max;

        this.released = false;
        this.duration = 1901;
        this.position = 0;
        this.velocity = 0;
        this.deacceleration = 0;
        this.speedThreshold = 0;

        this.timestamp = Date.now();
        this.lastPosition = 0;
        this.updateInterval = 15;

        this.callback = handler;
    };

    KineticModel.prototype.setPosition = function (pos)
    {
        this.released = false;
        if (pos === this.position) {
            return;
        }

        this.position = pos;
        this.position = Math.min(this.position, this.maximum);
        this.position = Math.max(this.position, this.minimum);

        this.callback(this.position);
        this.update(this);
        this.triggerUpdate(this.update);
    };

    KineticModel.prototype.triggerUpdate = function (f)
    {
        if (this.ticker === null) {
            var self = this;
            this.ticker = window.setInterval(function () {
                f(self);
            }, this.updateInterval);
        }
    };

    KineticModel.prototype.resetSpeed = function ()
    {
        this.velocity = 0;
        this.lastPosition = this.position;
    };

    KineticModel.prototype.release = function ()
    {
        this.released = true;
        this.deacceleration = Math.abs(this.velocity * 1000 / this.duration);
        this.speedThreshold = Math.abs(this.velocity * 0.3);
        this.triggerUpdate(this.update);
    };

    KineticModel.prototype.update = function (self)
    {
        var elapsed, delta, vstep, lastSpeed, currentSpeed;

        elapsed = Date.now() -  self.timestamp;
        if (isNaN(elapsed) || elapsed < 5) {
            return;
        }

        self.timestamp = Date.now();
        delta = elapsed / 1000;

        if (self.released) {
            self.position += (self.velocity * delta);
            self.position = Math.min(self.position, self.maximum);
            self.position = Math.max(self.position, self.minimum);
            vstep = self.deacceleration * delta;
            if (self.velocity > self.speedThreshold ||
                self.velocity < -self.speedThreshold) {
                vstep = 4 * vstep;
            }
            if (self.velocity < vstep && self.velocity > -vstep) {
                self.velocity = 0;
                self.position = Math.round(self.position);
                window.clearInterval(self.ticker);
                self.ticker = null;
            } else {
                self.velocity += (self.velocity < 0) ? vstep : -vstep;
            }
            self.callback(self.position);
        } else {
            lastSpeed = self.velocity;
            currentSpeed = (self.position - self.lastPosition) / delta;
            self.velocity = 0.23 * lastSpeed + 0.77 * currentSpeed;
            self.lastPosition = self.position;
        }
    };

    return KineticModel;

}());
