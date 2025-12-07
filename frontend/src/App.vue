<template>
  <div id="app">
    <h1>Welcome to Vue</h1>
    <p>{{ message }}</p>
  </div>
</template>

<script>
export default {
  name: 'App',
  data() {
    return {
      message: 'Hello Vue!'
    }
  }
}
</script>

<style scoped>
div {
  font-family: Arial, sans-serif;
  text-align: center;
  padding: 20px;
}
</style>